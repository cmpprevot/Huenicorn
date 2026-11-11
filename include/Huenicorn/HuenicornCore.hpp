#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <optional>
#include <thread>

#include <nlohmann/json.hpp>

#include <Huenicorn/ApiTools.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Config.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>
#include <Huenicorn/EntertainmentConfigurationSelector.hpp>
#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/IRestServer.hpp>
#include <Huenicorn/Streamer.hpp>
#include <Huenicorn/TickSynchronizer.hpp>
#include <Huenicorn/UV.hpp>
#include <Huenicorn/Interpolation.hpp>


namespace Huenicorn
{
  /**
   * @brief Main execution scope of Huenicorn that handles application components and executes streaming loop.
   * 
   */
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
     * @param version Current version of Huenicorn
     * @param configRoot Path to the configuration directory
     */
    HuenicornCore(const std::string& version, const std::filesystem::path& configRoot);


    // Getters
    /**
     * @brief Returns the current version of Huenicorn
     * 
     * @return const std::string& Current version of Huenicorn
     */
    const std::string& version() const;


    /**
     * @brief Returns whether the initial setup (bridge address + credentials) is complete
     * 
     * @return true Setup is complete
     * @return false Setup is still required
     */
    bool initialSetupOk() const;


    /**
     * @brief Returns whether the streaming loop is currently running
     *
     * @return true Streaming loop is running
     * @return false Streaming loop is not running
     */
    bool running() const;


    /**
     * @brief Returns whether ambilight streaming is currently active. The loop may be
     * running while ambilight is paused.
     *
     * @return true Ambilight is streaming colors to the bridge
     * @return false Ambilight is paused
     */
    bool streamingActive() const;


    /**
     * @brief Returns whether manual light control currently owns the stream.
     * In manual mode the stream still runs (it is the only way to address
     * gradient strip segments individually), but the channels are driven by
     * the manual overrides instead of the screen content. Ambilight and
     * manual mode are mutually exclusive.
     *
     * @return true Manual mode is active, ambilight following is suspended
     * @return false Ambilight drives the channels (when streaming is active)
     */
    bool manualMode() const;


    /**
     * @brief Returns whether the legacy web UI backend is enabled
     * 
     * @return true Web UI backend is enabled
     * @return false Web UI backend is disabled
     */
    bool webUIEnabled() const;


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
    std::optional<std::string> currentEntertainmentConfigurationId() const;


    /**
     * @brief Returns the resolution of the display
     * 
     * @return glm::ivec2 Display resolution
     */
    glm::ivec2 displayResolution() const;


    /**
     * @brief Returns a map of available subsample interpolations
     * 
     * @return const Interpolation::Interpolations& available interpolations
     */
    const Interpolation::Interpolations& availableInterpolations() const;


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
     * @brief Returns the current subsample interpolation type
     * 
     * @return Interpolation::Type Subsample interpolation type
     */
    Interpolation::Type interpolation() const;


    /**
     * @brief Returns the current transition smoothing factor
     *
     * @return float Transition smoothing factor, in percent [0, 100)
     */
    float transitionSmoothing() const;


    /**
     * @brief Returns the resolved Hue bridge IP address
     * 
     * @return nlohmann::json Object containing Hue bridge address and request status
     */
    nlohmann::json autodetectedBridge() const;


    /**
     * @brief Requests the addition of a new user on the Hue bridge
     * 
     * @return nlohmann::json Newly created user's credentials
     */
    nlohmann::json registerNewUser();


    // Setters
    /**
     * @brief Enables or disables the legacy web UI backend. When disabled, the initial
     * setup is expected to be driven by an external UI (e.g. the Qt GUI) and no browser
     * is spawned.
     * 
     * @param enabled Whether the web UI backend should be started
     */
    void setWebUIEnabled(bool enabled);


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


    /**
     * @brief Set the subsample interpolation 
     * 
     * @param interpolation Desired interpolation type
     */
    void setInterpolation(unsigned interpolation);


    /**
     * @brief Set the transition smoothing factor
     *
     * @param transitionSmoothing Desired smoothing amount, in percent [0, 100]
     */
    void setTransitionSmoothing(float transitionSmoothing);


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
     * @brief Activates or pauses ambilight streaming without stopping the application.
     * Pausing disables streaming on the bridge so lights leave entertainment mode;
     * resuming re-enables it. Any explicit streaming toggle ends manual mode and
     * drops its per-channel colors: resuming ambilight is the way to leave manual
     * mode.
     *
     * @param active true to stream colors, false to pause ambilight
     */
    void setStreamingActive(bool active);


    /**
     * @brief Applies a manual state (power, brightness, color or color temperature) to
     * the lights of the current entertainment configuration and enters manual mode:
     * screen following stops and every channel freezes at its last streamed color,
     * except the target channels which take the given state. In manual mode the
     * fixed colors are driven on the DTLS stream, which is the only way to address
     * gradient strip segments individually (the bridge maps channels to segments).
     * Resuming ambilight (setStreamingActive(true)) exits manual mode.
     *
     * @param state State to apply: {"on": bool, "brightness": 0-100,
     *              "r"/"g"/"b": 0-255; black when "on" is false},
     *              plus optional targeting keys: "lightId" (string, light service ID)
     *              targets a single light (all the channels holding it), otherwise
     *              "channelId" (int) targets only the lights attached to that
     *              channel, and when both are absent/empty all lights of the entertainment
     *              configuration are targeted
     * @return true State was applied to the target lights
     * @return false State could not be applied
     */
    bool setManualLightState(const nlohmann::json& state);


    /**
     * @brief Returns the last manual light state applied through setManualLightState
     *
     * @return nlohmann::json Last applied state, empty object if none was applied yet
     */
    nlohmann::json manualLightState() const;


    /**
     * @brief Returns the individual lights (light services) of the current
     * entertainment configuration, each listed once, with the lowest channel
     * holding it ("channelId", also its only one when "channelCount" is 1) and
     * the number of channels sharing it ("channelCount": e.g. 7 for a
     * multi-segment gradient strip, whose segments are addressed through the
     * channels - driving a single channel only matters when some light is
     * held by several ones)
     *
     * @return nlohmann::json Array of {lightId, name, channelId, channelCount}
     */
    nlohmann::json manualLights() const;


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


    /**
     * @brief Loads the last profile
     * 
     * @return Loaded profile. Empty if not found
     */
    std::optional<nlohmann::json> _getProfile();


    // Private methods

    /**
     * @brief Initializes config and profile
     * 
     * @return true Succeeded to load a suitable configuration
     * @return false Failed to load a suitable configuration
     */
    bool _initSettings();


    /**
     * @brief Starts the initial setup wizard
     *
     * @return true Setup finished success
     * @return false Setup was not completed
    */
    bool _runInitialSetup();


    /**
     * Brief Initializes a grabber based on the graphical session
     * 
     * @return true Relevant grabber was found and initialized
     * @return false could not initialize a grabber
    */
    bool _initGrabber();


    /**
     * Brief Initializes the web UI
    */
    void _initWebUI();


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
     * @brief Enables the selected entertainment configuration, recreates streamer  and loads related profile
     *
     * @param entertainmentConfigurationId ID of the entertainment configuration to enable
     */
    void _enableEntertainmentConfiguration(const std::string& entertainmentConfigurationId);


    /**
     * @brief Recreates the streamer for the current entertainment configuration.
     * A fresh DTLS handshake is performed, which is required after the bridge
     * tore down the previous session (e.g. when ambilight was paused).
     */
    void _recreateStreamer();


    /**
     * @brief Tears down the current streamer, closing its DTLS session. Must
     * be called before establishing a new session (the bridge keeps a single
     * entertainment session per client key and rejects new handshakes while it
     * is held), e.g. when switching to another entertainment configuration.
     */
    void _closeStreamer();


    /**
     * @brief Starts the main loop calling for processing at given refresh rate
     * 
     */
    void _startStreamingLoop();


    /**
     * @brief Computes the channels colors (from the grabbed frames, or from the
     * manual overrides in manual mode) and streams them to the bridge
     * 
     */
    void _update();


    /**
     * @brief Disables streaming on current entertainment configuration
     * 
     */
    void _shutdown();


    /**
     * @brief Called to match stream m_channelStreams size with m_channels
     * 
     */
    void _updateStreamChannelsSize();


    // Attributes
    const std::string& m_version;
    std::filesystem::path m_configRoot;
    Config m_config;

    std::atomic<bool> m_keepLooping{false};
    std::atomic<bool> m_stopRequested{false};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_streamingActive{true};
    std::atomic<bool> m_streamerStale{false};
    std::chrono::steady_clock::time_point m_lastStreamerRecreate{};
    bool m_webUIEnabled{true};
    bool m_openedSetup{false};
    std::unique_ptr<TickSynchronizer> m_tickSynchronizer;

    //  API structure wrapper
    std::unique_ptr<EntertainmentConfigurationSelector> m_selector;
    Channels m_channels;
    ChannelStreams m_channelStreams;

    // Streamer
    std::mutex m_streamerMutex;
    std::unique_ptr<Streamer> m_streamer;

    // Last manual light state applied through setManualLightState
    mutable std::mutex m_manualLightStateMutex;
    nlohmann::json m_manualLightState{nlohmann::json::object()};

    // Manual mode: when set, the stream is driven by the per-channel fixed
    // colors below instead of the grabbed screen content. Ambilight and manual
    // mode are mutually exclusive; setStreamingActive(false or true) clears it.
    std::atomic<bool> m_manualMode{false};

    // Per-channel fixed colors sent on the DTLS stream in manual mode. This is
    // the only way to address gradient strip segments individually: the bridge
    // maps channels to segments. Channels without an entry keep their last
    // streamed color (frozen). The values are sent as-is, without channel gamma.
    // Resuming ambilight clears these overrides.
    std::mutex m_manualOverrideMutex;
    std::unordered_map<uint8_t, glm::vec3> m_manualOverrides;

    // Service and flags
    ThreadedRestService m_webUIService;

    //  Image Processing
    SharedGrabber m_grabber;
    cv::Mat m_cvImage;
  };
}
