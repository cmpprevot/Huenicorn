#pragma once

#include <thread>
#include <optional>

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
  void _getHandler(const SharedSession& session);

  // Attributes
  SharedFreenSync m_freenSync;

  std::shared_ptr<restbed::Settings> m_settings;
  std::optional<std::thread> m_serviceThread;
  restbed::Service m_service;
};
