#pragma once

#include <optional>
#include <thread>

#include <nlohmann/json.hpp>

#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/IRestServer.hpp>
#include <Huenicorn/TickSynchronizer.hpp>
#include <Huenicorn/Config.hpp>
#include <Huenicorn/UV.hpp>
#include <Huenicorn/EntertainmentConfig.hpp>
#include <Huenicorn/EntertainmentConfigSelector.hpp>
#include <Huenicorn/Streamer.hpp>


namespace Huenicorn
{
  class HuenicornCore
  {
    struct ThreadedRestService
    {
      std::unique_ptr<IRestServer> server;
      std::optional<std::thread> thread;
    };

  public:
    // Constructor
    HuenicornCore(const std::filesystem::path& configRoot);


    // Getters
    const std::filesystem::path configFilePath() const;
    const Channels& channels() const;
    glm::ivec2 displayResolution() const;
    std::vector<glm::ivec2> subsampleResolutionCandidates() const;
    unsigned subsampleWidth() const;
    unsigned refreshRate() const;
    nlohmann::json autoDetectedBridge() const;
    nlohmann::json registerNewUser();


    // Setters
    const UVs& setChannelUV(uint8_t channelId, UV&& uv, UVType uvType);
    bool setChannelGammaFactor(uint8_t syncedChannelId, float gammaFactor);
    void setSubsampleWidth(unsigned subsampleWidth);
    void setRefreshRate(unsigned subsampleWidth);


    // Methods
    void start();
    void stop();
    bool validateBridgeAddress(const std::string& bridgeAddress);
    bool validateCredentials(const std::string& username, const std::string& clientkey);
    bool setChannelActivity(uint8_t channelId, bool active);
    void saveProfile() const;


  private:

    // Private methods
    bool _loadProfile();
    void _spawnBrowser();
    void _loop();
    void _processFrame();
    void _shutdownLights();


    // Attributes
    std::filesystem::path m_configRoot;
    std::filesystem::path m_profileFilePath;
    Config m_config;

    bool m_keepLooping;
    std::unique_ptr<TickSynchronizer> m_tickSynchronizer;

    //  Infrastructure
    std::unique_ptr<EntertainmentConfigSelector> m_selector;

    std::unique_ptr<Streamer> m_streamer;

    Channels m_channels;

    // Service and flags
    bool m_openedSetup{false};
    ThreadedRestService m_webUIService;

    //  Image Processing
    std::unique_ptr<IGrabber> m_grabber;
    cv::Mat m_cvImage;
  };
}
