#include <Huenicorn/HuenicornCore.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <Huenicorn/ScreenUtils.hpp>
#include <Huenicorn/RequestUtils.hpp>

using namespace nlohmann;
using namespace std;


namespace Huenicorn
{
  HuenicornCore::HuenicornCore():
  m_restServer(this)
  {
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
    m_bridge = make_shared<BridgeData>(m_config);

    if(!m_config.bridgeAddress().has_value()){
      if(!_registerBridgeAddress()){
        return;
      }
    }

    if(!m_config.apiKey().has_value()){
      if(!_registerApiToken()){
        return;
      }
    }

    if(m_config.subsampleWidth() == 0){
      m_config.setSubsampleWidth(ScreenUtils::subsampleResolutionCandidates().back().x);
    }

    cout << "Configuration is ready. Feel free to modify it manually by editing " << std::quoted(m_config.configFilePath().string()) << endl;

    m_restServer.start(m_config.restServerPort());
    m_keepLooping = true;
    _loadProfile();
    _loop();
  }


  void HuenicornCore::stop()
  {
    m_keepLooping = false;
    _shutdownLights();
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
    auto n = m_syncedLights.erase(lightId);
    _resetJsonLightsCache();
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

    ofstream profileFile("profile.json", ofstream::out);
    profileFile << profile.dump(2) << endl;
    profileFile.close();
  }


  bool HuenicornCore::_registerBridgeAddress()
  {
    while(!m_config.bridgeAddress().has_value()){
      cout << "A Hue bridge address is needed to enable light control. Are you ready to provide it ? [y/N]" << endl;
      string response;
      std::getline(std::cin, response);

      if(response != "y"){
        return false;
      }

      cout << "Please now provide the Hue bridge address" << endl;
      string bridgeAddress;
      std::getline(std::cin, bridgeAddress);

      try{
        auto response = RequestUtils::sendRequest(bridgeAddress + "/api", "GET", "");
        if(response.size() != 0){
          m_config.setBridgeAddress(bridgeAddress);
          cout << "Successfully registered bridge address" << endl;
        }
      }
      catch(const json::exception& exception){
        cout << "Could not register bridge address : " << std::quoted(bridgeAddress) << endl;
        cout << exception.what() << endl;
      }
    }

    return true;
  }


  bool HuenicornCore::_registerApiToken()
  {
    if(!m_config.apiKey().has_value()){
      cout << "An API token is required to interact with your Hue bridge.\n Can you provide any registered token ? [y/N]" << endl;

      string userResponse;
      std::getline(std::cin, userResponse);

      if(userResponse == "y"){
        size_t expectedTokenLength = 40;
        cout << "Please input a registered Hue Bridge API token (" << expectedTokenLength << " alphanumerical digits)" << endl;
        string userToken;
        std::getline(std::cin, userToken);

        if(userToken.size() != expectedTokenLength){
          cout << "Invalid token format" << endl;
          return false;
        }

        try{
          auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api/" + userToken, "GET", "");
          if(response.size() != 0){
            m_config.setApiKey(userToken);
            cout << "Successfully registered API token" << endl;
            return true;
          }
        }
        catch(const json::exception& exception){
          cout << "Could not register API token" << endl;
          cout << exception.what() << endl;
          return false;
        }
      }
      else{
        cout << "In order to generate a new token, you now need to press the central button on the Hue Bridge. Is it done and are you ready to proceed ? [y/N]" << endl;
        std::getline(std::cin, userResponse);
        if(userResponse == "y"){
          try{
            json request = {{"devicetype", "Huenicorn"}};

            auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

            if(response.size() < 1){
              return false;
            }

            if(response.at(0).contains("error")){
              cout << "Error : " << response.at(0).at("error").at("description") << endl;
              return false;
            }

            if(response.at(0).contains("success")){
              string apiToken = response.at(0).at("success").at("username");
              cout << "Api token " << std::quoted(apiToken) << "has been successfully generated !" << endl;
              m_config.setApiKey(apiToken);
              return true;
            }
          }
          catch(const json::exception& exception){
            return false;
          }
        }
        else{
          cout << "Aborting API token registration" << endl;
          return false;
        }
      }
    }

    return true;
  }


  void HuenicornCore::_loadProfile()
  {
    filesystem::path profilePath = "profile.json";

    if(!filesystem::exists(profilePath) || !filesystem::is_regular_file(profilePath)){
      cout << "No profile found yet." << endl;
      return;
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
  }


  void HuenicornCore::_loop()
  {
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
