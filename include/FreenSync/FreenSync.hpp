#pragma once

#include <optional>
#include <thread>

#include <FreenSync/ImageProcessing.hpp>
#include <FreenSync/BridgeData.hpp>
#include <FreenSync/ImageData.hpp>
#include <FreenSync/RestServer.hpp>

using SyncedLights = std::unordered_map<std::string, SharedSyncedLight>;
using SharedBridgeData = std::shared_ptr<BridgeData>;

class FreenSync
{
public:
  // Constructor
  FreenSync();


  // Getters
  const LightSummaries& availableLights() const;
  const SyncedLights& syncedLights() const;
  SharedSyncedLight syncedLight(const std::string& lightId) const;
  bool syncedLightExists(const std::string& lightId) const;
  glm::vec2 screenResolution() const;



  // Methods
  void start();
  void stop();
  SharedSyncedLight addSyncedLight(const std::string& lightId);
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


  //  Rest server
  RestServer m_restServer;
};
