#pragma once

#include <optional>
#include <thread>
#include <filesystem>

#include <nlohmann/json.hpp>

#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/IRestServer.hpp>
#include <Huenicorn/TickSynchronizer.hpp>
#include <Huenicorn/Config.hpp>
#include <Huenicorn/UV.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>
#include <Huenicorn/EntertainmentConfigurationSelector.hpp>
#include <Huenicorn/ApiTools.hpp>
#include <Huenicorn/Streamer.hpp>


namespace Huenicorn
{
  class HuenicornCore
  {
    // Type definitions
    struct ThreadedRestService
    {
      std::unique_ptr<IRestServer> server;
      std::optional<std::thread> thread;
    };

  public:
    // Constructor
    /**
     * @brief HuenicornCore constructor
     * 
     * @param configRoot 
     */
    HuenicornCore(const std::filesystem::path& configRoot);


    // Getters
    /**
     * @brief Returns the path of the configuration file
     * 
     * @return const std::filesystem::path Configuration file path
     */
    const std::filesystem::path configFilePath() const;


    /**
     * @brief Returns the list of the available channels
     * 
     * @return const Channels&  Channels list
     */
    const Channels& channels() const;


    /**
     * @brief Returns the list of available entertainment configurations
     * 
     * @return const EntertainmentConfigurations& Available entertainment configurations
     */
    const EntertainmentConfigurations& entertainmentConfigurations() const;


    /**
     * @brief Returns the current entertainment configuration
     * 
     * @return const EntertainmentConfiguration& Current entertainment configuration
     */
    const EntertainmentConfiguration& currentEntertainmentConfiguration() const;


    /**
     * @brief Returns the ID of the current entertainment configuration
     * 
     * @return const std::string& Current entertainment configuration ID
     */
    const std::string& currentEntertinmentConfigurationId() const;


    /**
     * @brief Returns the resolution of the display
     * 
     * @return glm::ivec2 Display resolution
     */
    glm::ivec2 displayResolution() const;


    /**
     * @brief Returns a list of subsample resolutions
     * 
     * @return std::vector<glm::ivec2> Subsample resolutions
     */
    std::vector<glm::ivec2> subsampleResolutionCandidates() const;


    /**
     * @brief Returns the current subsample width
     * 
     * @return unsigned Current subsample width
     */
    unsigned subsampleWidth() const;


    /**
     * @brief Returns the current color streaming refresh rate
     * 
     * @return unsigned Refresh rate
     */
    unsigned refreshRate() const;


    /**
     * @brief Returns the maximal refresh rate defined by the display
     * 
     * @return unsigned Maximal refresh rate
     */
    unsigned maxRefreshRate() const;


    /**
     * @brief Returns the resolved Hue bridge IP address
     * 
     * @return nlohmann::json Object containing Hue bridge address and request status
     */
    nlohmann::json autoDetectedBridge() const;


    /**
     * @brief Requests the addition of a new user on the Hue bridge
     * 
     * @return nlohmann::json Newly created user's credentials
     */
    nlohmann::json registerNewUser();


    // Setters
    /**
     * @brief Sets the current entertainment configuration
     * 
     * @param entertainmentConfigurationId ID of the entertainment configuration to select
     * @return true Successfully selected entertainment configuration
     * @return false Failed to select entertainment configuration
     */
    bool setEntertainmentConfiguration(const std::string& entertainmentConfigurationId);


    /**
     * @brief Set the Channel UV
     * 
     * @param channelId ID of the channel to affect
     * @param uv UV coordinate to set
     * @param uvCorner Corner to affect
     * @return const UVs& Clamped UV value
     */
    const UVs& setChannelUV(uint8_t channelId, UV&& uv, UVCorner uvCorner);


    /**
     * @brief Sets the Channel Gamma Factor
     * 
     * @param channelId ID of the channel to affect
     * @param gammaFactor Gamma factor to apply
     * @return true Gamma factor was successuflly set
     * @return false Gamma factor could not be set
     */
    bool setChannelGammaFactor(uint8_t channelId, float gammaFactor);


    /**
     * @brief Set the width of the subsample image to compute
     * 
     * @param subsampleWidth Desired width for the image subsample
     */
    void setSubsampleWidth(unsigned subsampleWidth);


    /**
     * @brief Set the refresh rate for color streaming
     * 
     * @param refreshRate Desired refresh rate for color data streaming
     */
    void setRefreshRate(unsigned refreshRate);


    // Methods
    /**
     * @brief Initiates Huenicorn and starts the main loop
     * 
     */
    void start();


    /**
     * @brief Stops the main loop
     * 
     */
    void stop();


    /**
     * @brief Registers Hue bridge address if valid
     * 
     * @param bridgeAddress Hue bridge address to register
     * @return true Hue bridge address is valid and was registered
     * @return false Hue bridge address is invalid and was discarded
     */
    bool validateBridgeAddress(const std::string& bridgeAddress);


    /**
     * @brief Registers Hue bridge credentials if valid
     * 
     * @param credentials Hue bridge credentials to register
     * @return true Hue bridge credentials are valid and were registered
     * @return false Hue bridge credentials are invalid and were discarded
     */
    bool validateCredentials(const Credentials& credentials);


    /**
     * @brief Set the streaming activity of the channel referenced by ID
     * 
     * @param channelId ID of the channel to affect
     * @param active State of the streaming
     * @return true Streaming state was successfully set
     * @return false Streaming state could not be set
     */
    bool setChannelActivity(uint8_t channelId, bool active);


    /**
     * @brief Saves the current state of a user-defined configuration as profile
     * 
     */
    void saveProfile();


  private:
    // Private getter
    /**
     * @brief Returns the path to the current profile
     * 
     * @return std::filesystem::path Path to the current profile
     */
    std::filesystem::path _profilePath() const;


    // Private methods
    /**
     * @brief Loads the last profile
     * 
     * @return true Profile was found and loaded
     * @return false Profile could not be loaded
     */
    bool _loadProfile();


    /**
     * @brief Initializes the channels based on the profile settings
     * 
     * @param jsonProfile Data from the user-defined profile
     */
    void _initChannels(const nlohmann::json& jsonProfile);


    /**
     * @brief Opens the user's default browser on the Huenicorn page
     * 
     */
    void _spawnBrowser();



    /**
     * @brief Enables the selected entertainment configuration and loads related profile
     * 
     */
    void _enableEntertainmentConfiguration();


    /**
     * @brief Starts the main loop calling for processing at given refresh rate
     * 
     */
    void _startStreamingLoop();


    /**
     * @brief Computes the channels colors and streams them
     * 
     */
    void _processFrame();


    /**
     * @brief Disables streaming on current entertainment configuration
     * 
     */
    void _shutdown();


    // Attributes
    std::filesystem::path m_configRoot;
    Config m_config;

    bool m_keepLooping;
    std::unique_ptr<TickSynchronizer> m_tickSynchronizer;

    //  API structure wrapper
    std::unique_ptr<EntertainmentConfigurationSelector> m_selector;
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
