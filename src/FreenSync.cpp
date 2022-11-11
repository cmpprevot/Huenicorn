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


void FreenSync::stop()
{
  if(!m_loopThread.has_value()){
    cout << "Service is not running" << endl;
    return;
  }

  m_keepLooping = false;
  m_loopThread.value().join();
}


const Lights& FreenSync::lights()
{
  return m_bridge.lights();
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

  auto& lights = m_bridge.lights();

  if(lights.size() == 0){
    cout << "No light found for test. Exiting" << endl;
    return;
  }

  m_keepLooping = true;
  while(m_keepLooping){
    _processScreenFrame();

    std::this_thread::sleep_for(1s / m_refreshRate);
  }
}


void FreenSync::_processScreenFrame()
{
  auto& lights = m_bridge.lights();

  ScreenUtils::getScreenCapture(m_imageData);
  int type = m_imageData.bitsPerPixel > 24 ? CV_8UC4 : CV_8UC3;
  cv::Mat img = cv::Mat(m_imageData.height, m_imageData.width, type, m_imageData.pixels.data());

  ImageProcessor::rescale(img, 100);

  int imgWidth = img.cols;
  int imgHeight = img.rows;


  for(const auto& [lightId, light] : lights){
    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;

    {
      std::lock_guard lock(m_uvMutex);
      const Light::UVs& uvs = light->uvs();

      /*
      x0 = uvs.first.x * imgWidth;
      y0 = uvs.first.y * imgHeight;
      x1 = uvs.second.x * imgWidth;
      y1 = uvs.second.y * imgHeight;
      */
      x0 = 0 * imgWidth;
      y0 = 0 * imgHeight;
      x1 = 0.5f * imgWidth;
      y1 = 0.5f * imgHeight;

    }

    cv::Mat subImage;
    ImageProcessor::getSubImage(img, x0, y0, x1, y1).copyTo(subImage);

    Colors colors = m_imageProcessor.getDominantColors(subImage, 1);

    if(lights.size() > 0){
      for(const auto& [lightId, light] : lights){
        light->setColor(colors.front());
      }
    }
  }
}
