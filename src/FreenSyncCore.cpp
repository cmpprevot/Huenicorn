#include <FreenSync/FreenSyncCore.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <FreenSync/ScreenUtils.hpp>
#include <FreenSync/TickSynchronizer.hpp>
#include <FreenSync/RequestUtils.hpp>

using namespace nlohmann;
using namespace std;


namespace FreenSync
{
  FreenSyncCore::FreenSyncCore():
  m_restServer(this)
  {
  }


  const LightSummaries& FreenSyncCore::availableLights() const
  {
    return m_bridge->lightSummaries();
  }


  const SyncedLights& FreenSyncCore::syncedLights() const
  {
    return m_syncedLights;
  }


  const nlohmann::json& FreenSyncCore::jsonAvailableLights() const
  {
    if(!m_cachedJsonAvailableLights.has_value()){
      m_cachedJsonAvailableLights.emplace(json::array());
      for(const auto& [key, light] : m_bridge->lightSummaries()){
        m_cachedJsonAvailableLights->push_back(light.serialize());
      }
    }

    return m_cachedJsonAvailableLights.value();
  }


  const nlohmann::json& FreenSyncCore::jsonSyncedLights() const
  {
    if(!m_cachedJsonSyncedLights.has_value()){
      m_cachedJsonSyncedLights.emplace(json::array());
      for(const auto& [key, light] : m_syncedLights){
        m_cachedJsonSyncedLights->push_back(light->serialize());
      }
    }
    return m_cachedJsonSyncedLights.value();
  }


  const nlohmann::json& FreenSyncCore::jsonAllLights() const
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


  SharedSyncedLight FreenSyncCore::syncedLight(const std::string& lightId) const
  {
    const auto& syncedLight = m_syncedLights.find(lightId);
    return (syncedLight != m_syncedLights.end()) ? syncedLight->second : nullptr;
  }


  glm::ivec2 FreenSyncCore::screenResolution() const
  {
    return ScreenUtils::getScreenResolution();
  }


  vector<glm::ivec2> FreenSyncCore::subsampleResolutionCandidates() const
  {
    return ScreenUtils::subsampleResolutionCandidates();
  }


  unsigned FreenSyncCore::subsampleWidth() const
  {
    return m_config.subsampleWidth();
  }


  const SyncedLight::UVs& FreenSyncCore::setLightUV(const std::string& syncedLightId, SyncedLight::UV&& uv, SyncedLight::UVType uvType)
  {
    _resetJsonLightsCache();
    return syncedLights().at(syncedLightId)->setUV(std::move(uv), uvType);
  }


  void FreenSyncCore::setLightGammaFactor(const std::string& syncedLightId, float gammaExponent)
  {
    m_syncedLights.at(syncedLightId)->setGammaFactor(gammaExponent);
  }


  void FreenSyncCore::setSubsampleWidth(int subsampleWidth)
  {
    m_config.setSubsampleWidth(subsampleWidth);
  }


  void FreenSyncCore::start()
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

    cout << "Configuration is ready. Feel free to modify it manually by editing " << std::quoted(m_config.configFilePath().string()) << endl;

    if(m_loopThread.has_value()){
      cout << "Service is already running" << endl;
      return;
    }

    m_restServer.start(m_config.restServerPort());
    m_keepLooping = true;
    m_refreshRate = m_config.refreshRate();
    m_loopThread.emplace([this](){_loop();});

    _loadProfile();
  }


  void FreenSyncCore::stop()
  {
    if(!m_loopThread.has_value()){
      cout << "Service is not running" << endl;
      return;
    }

    m_keepLooping = false;
    m_loopThread.value().join();

    _shutdownLights();
  }


  SharedSyncedLight FreenSyncCore::addSyncedLight(const std::string& lightId)
  {
    const auto& lightSummary = m_bridge->lightSummaries().at(lightId);
    auto [it, ok] = m_syncedLights.insert({lightId, make_shared<SyncedLight>(m_bridge, lightSummary)});

    _resetJsonLightsCache();
    return ok ? it->second : nullptr;
  }


  bool FreenSyncCore::removeSyncedLight(const std::string& lightId)
  {
    auto n = m_syncedLights.erase(lightId);
    _resetJsonLightsCache();
    return n > 0;
  }


  void FreenSyncCore::saveProfile() const
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


  bool FreenSyncCore::_registerBridgeAddress()
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


  bool FreenSyncCore::_registerApiToken()
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
            json request = {{"devicetype", "freenSync"}};

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


  void FreenSyncCore::_loadProfile()
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


  void FreenSyncCore::_loop()
  {
    TickSynchronizer ts(1.0f / m_refreshRate);

    ts.start();

    m_keepLooping = true;
    while(m_keepLooping){
      _processScreenFrame();

      if(!ts.sync()){
        cout << "Scheduled interval has been exceeded of " << ts.lastExcess().extra << " (" << ts.lastExcess().rate * 100 << "%)." << endl;
        cout << "Please reduce refreshRate if this warning persists." << endl;
      }
    }
  }


  void FreenSyncCore::_processScreenFrame()
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


  void FreenSyncCore::_shutdownLights()
  {
    for(const auto& [_, syncedLight] : m_syncedLights){
      syncedLight->setState(false);
    }
  }


  void FreenSyncCore::_resetJsonLightsCache()
  {
    m_cachedJsonAllLights.reset();
    m_cachedJsonAvailableLights.reset();
    m_cachedJsonSyncedLights.reset();
  }
}
