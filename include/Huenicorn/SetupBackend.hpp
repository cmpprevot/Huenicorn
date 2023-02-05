#pragma once

#include <Huenicorn/IRestServer.hpp>

#include <filesystem>


namespace Huenicorn
{
  class HuenicornCore;

  class SetupBackend : public IRestServer
  {
  public:
    SetupBackend(HuenicornCore* core);
    ~SetupBackend();

  // Getters
    bool aborted() const;

  protected:
    void _onStart() override;

  private:
    void _spawnBrowser();

    // Handlers
    void _finish(const SharedSession& session);
    void _abort(const SharedSession& session);
    void _autoDetectBridge(const SharedSession& session);
    void _configFilePath(const SharedSession& session);
    void _validateBridgeAddress(const SharedSession& session);
    void _validateApiKey(const SharedSession& session);
    void _requestNewApiKey(const SharedSession& session);

    // Attributes
    HuenicornCore* m_core;
    const std::filesystem::path m_webroot;
    std::unordered_map<std::string, std::string> m_contentTypes;
    bool m_aborted{false};
  };
}
