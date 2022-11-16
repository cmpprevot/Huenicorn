#include <FreenSync/FreenSync.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <FreenSync/ScreenUtils.hpp>

using namespace nlohmann;
using namespace std;


FreenSync::FreenSync()
{
  ifstream configFile("config.json");
  m_config = json::parse(configFile);

  m_bridge = make_shared<BridgeData>(m_config);
}


void FreenSync::start()
{
  if(m_loopThread.has_value()){
    cout << "Service is already running" << endl;
    return;
  }
  
  m_keepLooping = true;
  m_refreshRate = m_config.at("refreshRate");
  m_loopThread.emplace([this](){_loop();});

  _loadProfile();
}


bool FreenSync::addSyncedLight(const std::string& lightId)
{
  if(m_syncedLights.find(lightId) != m_syncedLights.end()){
    return false;
  }

  const auto& lightSummary = m_bridge->lightSummaries().at(lightId);

  m_syncedLights.insert({lightId, make_shared<SyncedLight>(m_bridge, lightId, m_bridge->lightSummaries().at(lightId))});

  return true;
}


void FreenSync::saveProfile() const
{
  nlohmann::json profile = json::object();
  profile["lights"] = json::array();
  for(const auto& [id, light] : m_syncedLights){
    const auto& uvs = light->uvs();
    profile["lights"].push_back({
      {"lightId", id},
      {"uvs", {
          {
            "uvA", {{"x", uvs.first.x}, {"y", uvs.first.y}}
          },
          {
            "uvB", {{"x", uvs.second.x}, {"y", uvs.second.y}}
          }
        }
      }
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

  for(const auto& light : jsonProfile.at("lights")){
    const string& lightId = light.at("lightId");
    if(lightSummaries.find(lightId) != lightSummaries.end()){
      m_syncedLights.insert({lightId, make_shared<SyncedLight>(m_bridge, lightId, lightSummaries.at(lightId))});
    }
  }
}


void FreenSync::stop()
{
  if(!m_loopThread.has_value()){
    cout << "Service is not running" << endl;
    return;
  }

  m_keepLooping = false;
  m_loopThread.value().join();

  _shutdownLights();
}


const LightSummaries& FreenSync::availableLights()
{
  return m_bridge->lightSummaries();
}


const SyncedLights& FreenSync::syncedLights()
{
  return m_syncedLights;
}


glm::vec2 FreenSync::screenResolution() const
{
  return ScreenUtils::getScreenResolution();
}


std::mutex& FreenSync::uvMutex()
{
  return m_uvMutex;
}


void FreenSync::_loop()
{
  m_keepLooping = true;
  while(m_keepLooping){
    _processScreenFrame();

    std::this_thread::sleep_for(1s / m_refreshRate);
  }
}


void FreenSync::_processScreenFrame()
{
  ScreenUtils::getScreenCapture(m_imageData);
  int type = m_imageData.bitsPerPixel > 24 ? CV_8UC4 : CV_8UC3;
  cv::Mat img = cv::Mat(m_imageData.height, m_imageData.width, type, m_imageData.pixels.data());

  ImageProcessor::rescale(img, 100);

  int imgWidth = img.cols;
  int imgHeight = img.rows;

  for(const auto& [lightId, light] : m_syncedLights){
    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;

    {
      std::lock_guard lock(m_uvMutex);
      const SyncedLight::UVs& uvs = light->uvs();

      x0 = uvs.first.x * imgWidth;
      y0 = uvs.first.y * imgHeight;
      x1 = uvs.second.x * imgWidth;
      y1 = uvs.second.y * imgHeight;
    }

    cv::Mat subImage;
    ImageProcessor::getSubImage(img, x0, y0, x1, y1).copyTo(subImage);

    Colors colors = ImageProcessor::getDominantColors(subImage, 1);

    for(const auto& [lightId, light] : m_syncedLights){
      light->setColor(colors.front());
    }
  }
}


void FreenSync::_shutdownLights()
{
  for(const auto& [lightId, syncedLight] : m_syncedLights){
    syncedLight->setState(false);
  }
}
