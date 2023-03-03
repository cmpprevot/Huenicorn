#include <Huenicorn/HuenicornCore.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <unistd.h>

#include <Huenicorn/X11Grabber.hpp>
#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/RequestUtils.hpp>
#include <Huenicorn/SetupBackend.hpp>
#include <Huenicorn/WebUIBackend.hpp>

#include <glm/trigonometric.hpp>

using namespace nlohmann;
using namespace glm;
using namespace std;


namespace Huenicorn
{
  HuenicornCore::HuenicornCore(const std::filesystem::path& configRoot):
  m_configRoot(configRoot),
  m_profileFilePath(m_configRoot / "profile.json"),
  m_config(m_configRoot),
  m_grabber(make_unique<X11Grabber>(&m_config))
  {}


  const std::filesystem::path HuenicornCore::configFilePath() const
  {
    return m_config.configFilePath();
  }


  /*
  const LightSummaries& HuenicornCore::availableLights() const
  {
    return m_bridge->lightSummaries();
  }


  const SyncedLights& HuenicornCore::syncedLights() const
  {
    return m_syncedLights;
  }
  */


  /*
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
  */


  /*
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
  */


  /*
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
  */


  /*
  SharedSyncedLight HuenicornCore::syncedLight(const std::string& lightId) const
  {
    const auto& syncedLight = m_syncedLights.find(lightId);
    return (syncedLight != m_syncedLights.end()) ? syncedLight->second : nullptr;
  }
  */


  glm::ivec2 HuenicornCore::screenResolution() const
  {
    return m_grabber->getScreenResolution();
  }


  vector<glm::ivec2> HuenicornCore::subsampleResolutionCandidates() const
  {
    return m_grabber->subsampleResolutionCandidates();
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
    string username = getlogin();
    string deviceType = "huenicorn#" + username;

    json request = {{"devicetype", deviceType}, {"generateclientkey", true}};
    auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

    if(response.size() < 1){
      return {{"succeeded", false}, {"error", "unreachable bridge"}};
    }

    if(response.at(0).contains("error")){
      return {{"succeeded", false}, {"error", response.at(0).at("error").at("description")}};
    }

    string apiToken = response.at(0).at("success").at("username");
    string clientKey = response.at(0).at("success").at("clientkey");
    m_config.setApiKey(apiToken);
    m_config.setClientkey(clientKey);

    return {{"succeeded", true}, {"apiKey", apiToken}};
  }


  const UVs& HuenicornCore::setChannelUV(const std::string& /*syncedChannelId*/, UV&& /*uv*/, UVType /*uvType*/)
  {
  /*
    _resetJsonLightsCache();
    return syncedLights().at(syncedLightId)->setUV(std::move(uv), uvType);
  */

    return tmpUvs; // Todo : remove
  }


  void HuenicornCore::setChannelGammaFactor(const std::string& /*syncedLightId*/, float /*gammaExponent*/)
  {
    //m_syncedLights.at(syncedLightId)->setGammaFactor(gammaExponent);
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

    //m_bridge = make_shared<BridgeData>(m_config);

    if(m_config.subsampleWidth() == 0){
      m_config.setSubsampleWidth(m_grabber->subsampleResolutionCandidates().back().x);
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


  /*
  SharedSyncedLight HuenicornCore::addSyncedLight(const std::string& lightId)
  {
    const auto& lightSummary = m_bridge->lightSummaries().at(lightId);
    auto [it, ok] = m_syncedLights.insert({lightId, make_shared<SyncedLight>(m_bridge, lightSummary)});

    _resetJsonLightsCache();
    return ok ? it->second : nullptr;
  }
  */


  bool HuenicornCore::removeSyncedLight(const std::string& /*lightId*/)
  {
    /*
    SharedSyncedLight tmpLight = m_syncedLights.at(lightId);
    auto n = m_syncedLights.erase(lightId);
    _resetJsonLightsCache();
    tmpLight->setState(false);
    return n > 0;
    */
    return false;
  }


  void HuenicornCore::saveProfile() const
  {
    /*
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
    */
  }


  bool HuenicornCore::_loadProfile()
  {
    /*
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
    */

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
    unsigned restServerPort = m_config.restServerPort();
    m_webUIService.server = make_unique<WebUIBackend>(this);
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(restServerPort);
    });

    if(!_loadProfile() && ! m_openedSetup){
      thread spawnBrowser([this](){_spawnBrowser();});
      spawnBrowser.detach();
    }

    const string& username = m_config.username().value();
    const string& clientkey = m_config.clientkey().value();
    const string& bridgeAddress =  m_config.bridgeAddress().value();

    m_selector = make_unique<EntertainmentConfigSelector>(username, clientkey, bridgeAddress);

    m_streamer = make_unique<Streamer>(m_config.username().value(), m_config.clientkey().value(), m_config.bridgeAddress().value());

    // Todo : select entertainment config prior to affect it
    m_streamer->setEntertainmentConfigId(m_selector->entertainmentConfigId());

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
    /*
    m_grabber->getScreenSubsample(m_cvImage);

    for(const auto& [_, light] : m_syncedLights){
      const SyncedLight::UVs& uvs = light->uvs();
      glm::ivec2 a{uvs.min.x * m_cvImage.cols, uvs.min.y * m_cvImage.rows};
      glm::ivec2 b{uvs.max.x * m_cvImage.cols, uvs.max.y * m_cvImage.rows};

      cv::Mat subImage;
      ImageProcessing::getSubImage(m_cvImage, a, b).copyTo(subImage);

      Colors colors = ImageProcessing::getDominantColors(subImage, 1);
      light->setColor(colors.front());
    }
    */

    
    // Begin Todo : Understand why the color from the screen is not good
    glm::ivec2 a{0, 0};
    glm::ivec2 b{m_cvImage.cols, m_cvImage.rows};
    m_grabber->getScreenSubsample(m_cvImage);
    cv::Mat subImage;
    ImageProcessing::getSubImage(m_cvImage, a, b).copyTo(subImage);
    Color color = ImageProcessing::getDominantColors(subImage, 1).front();

    vector<Channel> channels;

    glm::vec3 normalized = color.toNormalized(); // Main suspect is "toNormalized()"
    // End Todo


    channels.push_back({0, normalized.r, normalized.g, normalized.b}); // HARDCODED CHANNEL ID
    channels.push_back({1, normalized.r, normalized.g, normalized.b}); // HARDCODED CHANNEL ID
    m_streamer->streamChannels(channels);
  }


  void HuenicornCore::_shutdownLights()
  {
    /*
    for(const auto& [_, syncedLight] : m_syncedLights){
      syncedLight->setState(false);
    }
    */
  }


  void HuenicornCore::_resetJsonLightsCache()
  {
    //m_cachedJsonAllChannels.reset();
    //m_cachedJsonAvailableChannels.reset();
    //m_cachedJsonSyncedChannels.reset();
  }
}
