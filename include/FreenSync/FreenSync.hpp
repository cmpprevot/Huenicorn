#pragma once

#include <optional>
#include <thread>

#include <FreenSync/ImageProcessor.hpp>
#include <FreenSync/BridgeData.hpp>
#include <FreenSync/ImageData.hpp>

class FreenSync;
using SharedFreenSync = std::shared_ptr<FreenSync>;
using SyncedLights = std::unordered_map<std::string, SharedSyncedLight>;

using SharedBridgeData = std::shared_ptr<BridgeData>;

class FreenSync
{
public:
  FreenSync();

  void start();

  void stop();


  // Getters
  const LightSummaries& availableLights();
  const SyncedLights& syncedLights();


  glm::vec2 screenResolution() const;
  std::mutex& uvMutex();

  bool addSyncedLight(const std::string& lightId);
  void saveProfile() const;

private:

  // Private methods
  void _loadProfile();
  void _loop();
  void _processScreenFrame();
  void _shutdownLights();


  // Attributes
  nlohmann::json m_config;

  std::optional<std::thread> m_loopThread;
  bool m_keepLooping;
  float m_refreshRate;

  //  Infrastructure
  SharedBridgeData m_bridge;

  SyncedLights m_syncedLights;

  //  Image Processing
  ImageData m_imageData;

  std::mutex m_uvMutex;

};
