#include <FreenSync/FreenSync.hpp>

#include <iostream>
#include <chrono>

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
  ImageProcessor::getScreenCapture(m_imageData);
  int type = m_imageData.bitsPerPixel > 24 ? CV_8UC4 : CV_8UC3;
  cv::Mat img = cv::Mat(m_imageData.height, m_imageData.width, type, m_imageData.pixels.data());


  ImageProcessor::rescale(img, 100);

  Colors colors = m_imageProcessor.getDominantColors(img, 1);

  auto& lights = m_bridge.lights();

  if(lights.size() > 0){
    auto& light = lights.back();
    light->setColor(colors.front());
  }
}
