#pragma once

#include <optional>
#include <thread>

#include <FreenSync/ImageProcessor.hpp>
#include <FreenSync/BridgeData.hpp>
#include <FreenSync/ImageData.hpp>

class FreenSync;
using SharedFreenSync = std::shared_ptr<FreenSync>;
using SyncedLights = std::unordered_map<std::string, SharedSyncedLight>;

class FreenSync
{
public:
  void start(float refreshRate);

  void stop();


  // Getters
  const LightSummaries& availableLights();
  const SyncedLights& syncedLights();


  glm::vec2 screenResolution() const;
  std::mutex& uvMutex();

  bool addSyncedLight(const std::string& lightId);

private:

  void _loop();
  void _processScreenFrame();

  std::optional<std::thread> m_loopThread;
  bool m_keepLooping;
  float m_refreshRate;

  // Infrastructure
  BridgeData m_bridge;

  SyncedLights m_syncedLights;

  // Image Processing
  ImageProcessor m_imageProcessor;
  ImageData m_imageData;

  std::mutex m_uvMutex;

};
