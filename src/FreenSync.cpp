#include <FreenSync/FreenSync.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <FreenSync/ScreenUtils.hpp>
#include <FreenSync/TickSynchronizer.hpp>

using namespace nlohmann;
using namespace std;


FreenSync::FreenSync():
m_restServer(this)
{
  ifstream configFile("config.json");
  m_config = json::parse(configFile);

  m_bridge = make_shared<BridgeData>(m_config);

  m_restServer.start(m_config.at("restServerPort"));
}


const LightSummaries& FreenSync::availableLights() const
{
  return m_bridge->lightSummaries();
}


const SyncedLights& FreenSync::syncedLights() const
{
  return m_syncedLights;
}


const nlohmann::json& FreenSync::jsonAvailableLights() const
{
  if(!m_cachedJsonAvailableLights.has_value()){
    m_cachedJsonAvailableLights.emplace(json::array());
    for(const auto& [key, light] : m_bridge->lightSummaries()){
      m_cachedJsonAvailableLights->push_back(light.serialize());
    }
  }

  return m_cachedJsonAvailableLights.value();
}


const nlohmann::json& FreenSync::jsonSyncedLights() const
{
  if(!m_cachedJsonSyncedLights.has_value()){
    m_cachedJsonSyncedLights.emplace(json::array());
    for(const auto& [key, light] : m_syncedLights){
      m_cachedJsonSyncedLights->push_back(light->serialize());
    }
  }
  return m_cachedJsonSyncedLights.value();
}


const nlohmann::json& FreenSync::jsonAllLights() const
{
  if(!m_cachedJsonAllLights.has_value()){

    json test = {
      {"a", 42}
    };

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


SharedSyncedLight FreenSync::syncedLight(const std::string& lightId) const
{
  const auto& syncedLight = m_syncedLights.find(lightId);
  return (syncedLight != m_syncedLights.end()) ? syncedLight->second : nullptr;
}


glm::vec2 FreenSync::screenResolution() const
{
  return ScreenUtils::getScreenResolution();
}


unsigned FreenSync::subsampleWidth() const
{
  return m_config.at("subsampleWidth");
}


const SyncedLight::UVs& FreenSync::setLightUV(const std::string& syncedLightId, SyncedLight::UV&& uv, SyncedLight::UVType uvType)
{
  _resetJsonLightsCache();
  return syncedLights().at(syncedLightId)->setUV(std::move(uv), uvType);
}


void FreenSync::setLightGammaFactor(const std::string& syncedLightId, float gammaExponent)
{
  m_syncedLights.at(syncedLightId)->setGammaFactor(gammaExponent);
}


void FreenSync::start()
{
  if(m_loopThread.has_value()){
    cout << "Service is already running" << endl;
    return;
  }

  m_keepLooping = true;
  m_refreshRate = m_config.at("refreshRate").get<float>();
  m_loopThread.emplace([this](){_loop();});

  _loadProfile();
}


void FreenSync::stop()
{
  if(!m_loopThread.has_value()){
    cout << "Service is not running" << endl;
    return;
  }

  m_keepLooping = false;
  m_loopThread.value().join();

  m_restServer.stop();

  _shutdownLights();
}


SharedSyncedLight FreenSync::addSyncedLight(const std::string& lightId)
{
  const auto& lightSummary = m_bridge->lightSummaries().at(lightId);
  auto [it, ok] = m_syncedLights.insert({lightId, make_shared<SyncedLight>(m_bridge, lightSummary)});

  _resetJsonLightsCache();
  return ok ? it->second : nullptr;
}


bool FreenSync::removeSyncedLight(const std::string& lightId)
{
  auto n = m_syncedLights.erase(lightId);
  _resetJsonLightsCache();
  return n > 0;
}


void FreenSync::saveProfile() const
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


void FreenSync::_loadProfile()
{
  filesystem::path profilePath = "profile.json";

  if(!filesystem::exists(profilePath) || !filesystem::is_regular_file(profilePath)){
    cout << "No profile found yet." << endl;
    return;
  }

  ifstream profileFile(profilePath);
  json jsonProfile = json::parse(profileFile);
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


void FreenSync::_loop()
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


void FreenSync::_processScreenFrame()
{
  ScreenUtils::getScreenCapture(m_imageData);
  int type = m_imageData.bitsPerPixel > 24 ? CV_8UC4 : CV_8UC3;
  cv::Mat img = cv::Mat(m_imageData.height, m_imageData.width, type, m_imageData.pixels.data());

  ImageProcessing::rescale(img, m_config.at("subsampleWidth").get<unsigned>());

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


void FreenSync::_shutdownLights()
{
  for(const auto& [_, syncedLight] : m_syncedLights){
    syncedLight->setState(false);
  }
}


void FreenSync::_resetJsonLightsCache()
{
  m_cachedJsonAllLights.reset();
  m_cachedJsonAvailableLights.reset();
  m_cachedJsonSyncedLights.reset();
}
