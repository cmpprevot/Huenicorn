#pragma once

#include <thread>
#include <optional>
#include <filesystem>

#include <restbed>

class FreenSync;
using SharedFreenSync = std::shared_ptr<FreenSync>;

class RestServer
{
  using SharedSession = std::shared_ptr<restbed::Session>;

public:
  RestServer(SharedFreenSync freenSync, int port);

  bool start();
  bool stop();

private:
  // Handlera
  void _getLights(const SharedSession& session) const;
  void _getScreen(const SharedSession& session) const;
  void _getWebFile(const SharedSession& session) const;
  void _setLightUVs(const SharedSession& session) const;

  // Attributes
  SharedFreenSync m_freenSync;

  std::shared_ptr<restbed::Settings> m_settings;
  std::optional<std::thread> m_serviceThread;
  restbed::Service m_service;

  const std::filesystem::path m_webroot;
  std::unordered_map<std::string, std::string> m_contentTypes;
};
