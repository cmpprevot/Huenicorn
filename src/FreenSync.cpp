#include <FreenSync/FreenSync.hpp>

#include <iostream>
#include <chrono>

#include <FreenSync/ScreenUtils.hpp>

using namespace std;


void FreenSync::start(float refreshRate)
{
  if(m_loopThread.has_value()){
    cout << "Service is already running" << endl;
    return;
  }
  
  m_keepLooping = true;
  m_refreshRate = refreshRate;
  m_loopThread.emplace([this](){_loop();});
}


bool FreenSync::addSyncedLight(const std::string& lightId)
{
  if(m_syncedLights.find(lightId) != m_syncedLights.end()){
    return false;
  }

  const auto& lightSummary = m_bridge.lightSummaries().at(lightId);

  m_syncedLights.insert({lightId, make_shared<SyncedLight>(&m_bridge, lightId, m_bridge.lightSummaries().at(lightId))});

  return true;
}


void FreenSync::stop()
{
  if(!m_loopThread.has_value()){
    cout << "Service is not running" << endl;
    return;
  }

  m_keepLooping = false;
  m_loopThread.value().join();
}


const LightSummaries& FreenSync::availableLights()
{
  return m_bridge.lightSummaries();
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

    Colors colors = m_imageProcessor.getDominantColors(subImage, 1);

    for(const auto& [lightId, light] : m_syncedLights){
      light->setColor(colors.front());
    }
  }
}
