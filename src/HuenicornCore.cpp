#include <Huenicorn/HuenicornCore.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <Huenicorn/ScreenUtils.hpp>
#include <Huenicorn/RequestUtils.hpp>
#include <Huenicorn/SetupBackend.hpp>

using namespace nlohmann;
using namespace std;


namespace Huenicorn
{
  HuenicornCore::HuenicornCore(const std::filesystem::path& configRoot):
  m_configRoot(configRoot),
  m_profileFilePath(m_configRoot / "profile.json"),
  m_config(m_configRoot)
  {
  }
  
  
  const std::filesystem::path HuenicornCore::configFilePath() const
  {
    return m_config.configFilePath();
  }


  const LightSummaries& HuenicornCore::availableLights() const
  {
    return m_bridge->lightSummaries();
  }


  const SyncedLights& HuenicornCore::syncedLights() const
  {
    return m_syncedLights;
  }


  const nlohmann::json& HuenicornCore::jsonAvailableLights() const
  {
    if(!m_cachedJsonAvailableLights.has_value()){
      m_cachedJsonAvailableLights.emplace(json::array());
      for(const auto& [key, light] : m_bridge->lightSummaries()){
        m_cachedJsonAvailableLights->push_back(light.serialize());
      }
    }

    return m_cachedJsonAvailableLights.value();
  }


  const nlohmann::json& HuenicornCore::jsonSyncedLights() const
  {
    if(!m_cachedJsonSyncedLights.has_value()){
      m_cachedJsonSyncedLights.emplace(json::array());
      for(const auto& [key, light] : m_syncedLights){
        m_cachedJsonSyncedLights->push_back(light->serialize());
      }
    }
    return m_cachedJsonSyncedLights.value();
  }


  const nlohmann::json& HuenicornCore::jsonAllLights() const
  {
    if(!m_cachedJsonAllLights.has_value()){
      m_cachedJsonAllLights.emplace(
        json::object(
          {
            {"synced", jsonSyncedLights()},
            {"available", jsonAvailableLights()}
          }
        )
      );
    }

    return m_cachedJsonAllLights.value();
  }


  SharedSyncedLight HuenicornCore::syncedLight(const std::string& lightId) const
  {
    const auto& syncedLight = m_syncedLights.find(lightId);
    return (syncedLight != m_syncedLights.end()) ? syncedLight->second : nullptr;
  }


  glm::ivec2 HuenicornCore::screenResolution() const
  {
    return ScreenUtils::getScreenResolution();
  }


  vector<glm::ivec2> HuenicornCore::subsampleResolutionCandidates() const
  {
    return ScreenUtils::subsampleResolutionCandidates();
  }


  unsigned HuenicornCore::subsampleWidth() const
  {
    return m_config.subsampleWidth();
  }


  unsigned HuenicornCore::refreshRate() const
  {
    return m_config.refreshRate();
  }


  unsigned HuenicornCore::transitionTime_c() const
  {
    return m_config.transitionTime_c();
  }


  json HuenicornCore::autoDetectedBridge() const
  {
    string bridgeAddress;
    try{
      auto detectedBridgeData = RequestUtils::sendRequest("https://discovery.meethue.com/", "GET");

      if(detectedBridgeData.size() < 1){
        return {{"succeeded", false}, {"error", "Could not autodetect bridge."}};
      }

     bridgeAddress = detectedBridgeData.front().at("internalipaddress");
    }
    catch(const json::exception& e){
      return {{"succeeded", false}, {"error", "Could not autodetect bridge."}};
    }

    return {{"succeeded", bridgeAddress != ""}, {"bridgeAddress", bridgeAddress}};
  }


  json HuenicornCore::requestNewApiKey()
  {
    json request = {{"devicetype", "Huenicorn"}};
    auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

    if(response.size() < 1){
      return {{"succeeded", false}, {"error", "unreachable bridge"}};
    }

    if(response.at(0).contains("error")){
      return {{"succeeded", false}, {"error", response.at(0).at("error").at("description")}};
    }

    string apiToken = response.at(0).at("success").at("username");
    m_config.setApiKey(apiToken);

    return {{"succeeded", true}, {"apiKey", apiToken}};
  }


  const SyncedLight::UVs& HuenicornCore::setLightUV(const std::string& syncedLightId, SyncedLight::UV&& uv, SyncedLight::UVType uvType)
  {
    _resetJsonLightsCache();
    return syncedLights().at(syncedLightId)->setUV(std::move(uv), uvType);
  }


  void HuenicornCore::setLightGammaFactor(const std::string& syncedLightId, float gammaExponent)
  {
    m_syncedLights.at(syncedLightId)->setGammaFactor(gammaExponent);
  }


  void HuenicornCore::setSubsampleWidth(unsigned subsampleWidth)
  {
    m_config.setSubsampleWidth(subsampleWidth);
  }


  void HuenicornCore::setRefreshRate(unsigned refreshRate)
  {
    m_config.setRefreshRate(refreshRate);
    refreshRate = m_config.refreshRate();
    m_tickSynchronizer->setTickInterval(1.0f / refreshRate);
  }


  void HuenicornCore::setTransitionTime_c(unsigned transitionTime_c)
  {
    m_config.setTransitionTime_c(transitionTime_c);
  }


  void HuenicornCore::start()
  {
    unsigned port = m_config.restServerPort();

    if(!m_config.initialSetupOk()){
      cout << "Starting setup backend" << endl;

      SetupBackend sb(this);
      sb.start(port);

      if(sb.aborted()){
        cout << "Initial setup was aborted" << endl;
        return;
      }

      cout << "Finished setup" << endl;
      m_openedSetup = true;
    }

    if(!m_config.initialSetupOk()){
      cout << "There are errors in the config file" << endl;
      return;
    }

    m_bridge = make_shared<BridgeData>(m_config);

    if(m_config.subsampleWidth() == 0){
      m_config.setSubsampleWidth(ScreenUtils::subsampleResolutionCandidates().back().x);
    }

    cout << "Configuration is ready. Feel free to modify it manually by editing " << std::quoted(m_config.configFilePath().string()) << endl;

    _loop();
  }


  void HuenicornCore::stop()
  {
    m_keepLooping = false;
  }


  bool HuenicornCore::validateBridgeAddress(const std::string& bridgeAddress)
  {
    try{
      string sanitizedAddress = bridgeAddress;
      while (sanitizedAddress.back() == '/'){
        sanitizedAddress.pop_back();
      }

      auto response = RequestUtils::sendRequest(sanitizedAddress + "/api", "GET", "");
      if(response.size() == 0){
        return false;
      }

      m_config.setBridgeAddress(sanitizedAddress);
    }
    catch(const json::exception& exception){
      cout << exception.what() << endl;
      return false;
    }

    return true;
  }


  bool HuenicornCore::validateApiKey(const std::string& apiKey)
  {
    try{
      auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api/" + apiKey, "GET", "");
      if(response.size() == 0){
        return false;
      }

      if(response.is_array() && response.at(0).contains("error")){
        return false;
      }

      m_config.setApiKey(apiKey);
      cout << "Successfully registered API key" << endl;
    }
    catch(const json::exception& exception){
      cout << exception.what() << endl;
      return false;
    }

    return true;
  }


  SharedSyncedLight HuenicornCore::addSyncedLight(const std::string& lightId)
  {
    const auto& lightSummary = m_bridge->lightSummaries().at(lightId);
    auto [it, ok] = m_syncedLights.insert({lightId, make_shared<SyncedLight>(m_bridge, lightSummary)});

    _resetJsonLightsCache();
    return ok ? it->second : nullptr;
  }


  bool HuenicornCore::removeSyncedLight(const std::string& lightId)
  {
    SharedSyncedLight tmpLight = m_syncedLights.at(lightId);
    auto n = m_syncedLights.erase(lightId);
    _resetJsonLightsCache();
    tmpLight->setState(false);
    return n > 0;
  }


  void HuenicornCore::saveProfile() const
  {
    nlohmann::json profile = json::object();
    profile["lights"] = json::array();
    for(const auto& [id, light] : m_syncedLights){
      const auto& uvs = light->uvs();
      profile["lights"].push_back({
        {"id", id},
        {"uvs", {
            {
              "uvA", {{"x", uvs.min.x}, {"y", uvs.min.y}}
            },
            {
              "uvB", {{"x", uvs.max.x}, {"y", uvs.max.y}}
            }
          }
        },
        {"gammaFactor", light->gammaFactor()}
      });
    }

    ofstream profileFile(m_profileFilePath, ofstream::out);
    profileFile << profile.dump(2) << endl;
    profileFile.close();
  }


  bool HuenicornCore::_loadProfile()
  {
    filesystem::path profilePath = m_profileFilePath;

    if(!filesystem::exists(profilePath) || !filesystem::is_regular_file(profilePath)){
      cout << "No profile found yet." << endl;
      return false;
    }

    ifstream profileFile(profilePath);
    json jsonProfile = json::parse(profileFile);

    // ToDo : Try/catch
    const auto& lightSummaries = m_bridge->lightSummaries();

    for(const auto& jsonLight : jsonProfile.at("lights")){
      const string& lightId = jsonLight.at("id");
      if(lightSummaries.find(lightId) != lightSummaries.end()){
        SharedSyncedLight newSyncedLight = addSyncedLight(lightId);

        json jsonUVs = jsonLight.at("uvs");
        float uvAx = jsonUVs.at("uvA").at("x");
        float uvAy = jsonUVs.at("uvA").at("y");
        float uvBx = jsonUVs.at("uvB").at("x");
        float uvBy = jsonUVs.at("uvB").at("y");

        newSyncedLight->setUV({uvAx, uvAy}, SyncedLight::UVType::TopLeft);
        newSyncedLight->setUV({uvBx, uvBy}, SyncedLight::UVType::BottomRight);
        newSyncedLight->setGammaFactor(jsonLight.at("gammaFactor"));
        m_syncedLights.insert({lightId, newSyncedLight});
      }
    }

    return true;
  }


  void HuenicornCore::_spawnBrowser()
  {
    while (!m_webUIService.server->running()){
      this_thread::sleep_for(100ms);
    }
    
    stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_config.restServerPort();
    string serviceURL = serviceUrlStream.str();
    std::cout << "Management WebUI is ready and available at " << serviceURL << std::endl;

    system(string("xdg-open " + serviceURL).c_str());
  }


  void HuenicornCore::_loop()
  {
    unsigned port = m_config.restServerPort();
    m_webUIService.server = make_unique<WebUIBackend>(this);
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(port);
    });

    if(!_loadProfile() && ! m_openedSetup){
      thread spawnBrowser([this](){_spawnBrowser();});
      spawnBrowser.detach();
    }

    m_tickSynchronizer = make_unique<TickSynchronizer>(1.0f / static_cast<float>(m_config.refreshRate()));

    m_tickSynchronizer->start();

    m_keepLooping = true;
    while(m_keepLooping){
      _processScreenFrame();

      if(!m_tickSynchronizer->sync()){
        cout << "Scheduled interval has been exceeded of " << m_tickSynchronizer->lastExcess().extra << " (" << m_tickSynchronizer->lastExcess().rate * 100 << "%)." << endl;
        cout << "Please reduce refreshRate if this warning persists." << endl;
      }
    }

    _shutdownLights();

    m_webUIService.server->stop();
    m_webUIService.thread.value().join();
  }


  void HuenicornCore::_processScreenFrame()
  {
    ScreenUtils::getScreenCapture(m_imageData);
    int type = m_imageData.bitsPerPixel > 24 ? CV_8UC4 : CV_8UC3;
    cv::Mat img = cv::Mat(m_imageData.height, m_imageData.width, type, m_imageData.pixels.data());

    ImageProcessing::rescale(img, m_config.subsampleWidth());

    cv::cvtColor(img, img, cv::COLOR_RGBA2RGB);

    int imgWidth = img.cols;
    int imgHeight = img.rows;

    for(const auto& [_, light] : m_syncedLights){
      const SyncedLight::UVs& uvs = light->uvs();
      glm::ivec2 a{uvs.min.x * imgWidth, uvs.min.y * imgHeight};
      glm::ivec2 b{uvs.max.x * imgWidth, uvs.max.y * imgHeight};

      cv::Mat subImage;
      ImageProcessing::getSubImage(img, a, b).copyTo(subImage);

      Colors colors = ImageProcessing::getDominantColors(subImage, 1);
      light->setColor(colors.front());
    }
  }


  void HuenicornCore::_shutdownLights()
  {
    for(const auto& [_, syncedLight] : m_syncedLights){
      syncedLight->setState(false);
    }
  }


  void HuenicornCore::_resetJsonLightsCache()
  {
    m_cachedJsonAllLights.reset();
    m_cachedJsonAvailableLights.reset();
    m_cachedJsonSyncedLights.reset();
  }
}
