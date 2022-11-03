#pragma once

#include <optional>
#include <thread>

#include <FreenSync/ImageProcessor.hpp>
#include <FreenSync/BridgeData.hpp>
#include <FreenSync/ImageData.hpp>

class FreenSync;
using SharedFreenSync = std::shared_ptr<FreenSync>;

class FreenSync
{
public:
  void start(float refreshRate);

  void stop();


  // Getters
  const Lights& lights();
  glm::vec2 screenResolution() const;


private:

  void _loop();
  void _processScreenFrame();

  std::optional<std::thread> m_loopThread;
  bool m_keepLooping;
  float m_refreshRate;

  // Infrastructure
  BridgeData m_bridge;

  // Image Processing
  ImageProcessor m_imageProcessor;
  ImageData m_imageData;
};
