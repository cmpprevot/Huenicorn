#pragma once

#include <thread>
#include <optional>
#include <filesystem>

#include <restbed>

class FreenSync;

class RestServer;
using SharedRestServer = std::shared_ptr<RestServer>;

class RestServer
{
  using SharedSession = std::shared_ptr<restbed::Session>;

public:
  RestServer(FreenSync* freenSync);

  bool start(int port);
  bool stop();

private:
  // Handlers
  void _getAvailableLights(const SharedSession& session) const;
  void _getSyncedLights(const SharedSession& session) const;
  void _getAllLights(const SharedSession& session) const;
  void _getScreen(const SharedSession& session) const;
  void _getWebFile(const SharedSession& session) const;
  void _setLightUV(const SharedSession& session) const;
  void _syncLight(const SharedSession& session) const;
  void _saveProfile(const SharedSession& session) const;

  // Attributes
  FreenSync* m_freenSync;

  std::shared_ptr<restbed::Settings> m_settings;
  std::optional<std::thread> m_serviceThread;
  restbed::Service m_service;

  const std::filesystem::path m_webroot;
  std::unordered_map<std::string, std::string> m_contentTypes;
};
