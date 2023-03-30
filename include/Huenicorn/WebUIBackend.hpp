#pragma once

#include <Huenicorn/IRestServer.hpp>

#include <filesystem>

namespace Huenicorn
{
  class HuenicornCore;

  class WebUIBackend : public IRestServer
  {
    // Type definitions
    using SharedSession = std::shared_ptr<restbed::Session>;

  public:
    // Constructor
    WebUIBackend(HuenicornCore* HuenicornCore);

  private:

    // Handlers
    void _entertainmentConfigurations(const SharedSession& session) const;
    void _getChannel(const SharedSession& session) const;
    void _getChannels(const SharedSession& session) const;
    void _getChannelsMembers(const SharedSession& session) const;
    void _getDisplayInfo(const SharedSession& session) const;
    void _setEntertainmentConfiguration(const SharedSession& session) const;
    void _setChannelUV(const SharedSession& session) const;
    void _setChannelGammaFactor(const SharedSession& session) const;
    void _setSubsampleWidth(const SharedSession& session) const;
    void _setRefreshRate(const SharedSession& session) const;
    void _setChannelActivity(const SharedSession& session) const;
    void _saveProfile(const SharedSession& session) const;
    void _stop(const SharedSession& session) const;

    // Attributes
    HuenicornCore* m_huenicornCore;
  };
}
