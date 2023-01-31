#pragma once

#include <Huenicorn/IRestServer.hpp>

#include <filesystem>

#include <restbed>

namespace Huenicorn
{
  class HuenicornCore;

  class WebUIBackend : public IRestServer
  {
    using SharedSession = std::shared_ptr<restbed::Session>;

  public:
    WebUIBackend(HuenicornCore* HuenicornCore);

  private:

    // Handlers
    void _getAvailableLights(const SharedSession& session) const;
    void _getSyncedLights(const SharedSession& session) const;
    void _getSyncedLight(const SharedSession& session) const;
    void _getAllLights(const SharedSession& session) const;
    void _getDisplayInfo(const SharedSession& session) const;
    void _getTransitionTime_c(const SharedSession& session) const;
    void _setLightUV(const SharedSession& session) const;
    void _setLightGammaFactor(const SharedSession& session) const;
    void _setSubsampleWidth(const SharedSession& session) const;
    void _setRefreshRate(const SharedSession& session) const;
    void _setTransitionTime_c(const SharedSession& session) const;
    void _syncLight(const SharedSession& session) const;
    void _unsyncLight(const SharedSession& session) const;
    void _saveProfile(const SharedSession& session) const;

    // Attributes
    HuenicornCore* m_huenicornCore;

  };
}
