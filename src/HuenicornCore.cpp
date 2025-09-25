#include <Huenicorn/HuenicornCore.hpp>

#include <fstream>
#include <chrono>

#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/Interpolation.hpp>
#include <Huenicorn/Logger.hpp>
#include <Huenicorn/HttpRequestUtils.hpp>
#include <Huenicorn/SetupBackend.hpp>
#include <Huenicorn/WebUIBackend.hpp>
#include <Huenicorn/DummyGrabber.hpp>
#include <Huenicorn/PlatformSelector.hpp>


using namespace std::chrono_literals;


namespace Huenicorn
{
  HuenicornCore::HuenicornCore(const std::string& version, const std::filesystem::path& configRoot):
  m_version(version),
  m_configRoot(configRoot),
  m_config(m_configRoot)
  {}


  const std::string& HuenicornCore::version() const
  {
    return m_version;
  }


  const std::filesystem::path HuenicornCore::configFilePath() const
  {
    return m_config.configFilePath();
  }


  const Channels& HuenicornCore::channels() const
  {
    return m_channels;
  }


  const EntertainmentConfigurations& HuenicornCore::entertainmentConfigurations() const
  {
    return m_selector->entertainmentConfigurations();
  }


  const EntertainmentConfiguration& HuenicornCore::currentEntertainmentConfiguration() const
  {
    return m_selector->currentEntertainmentConfiguration();
  }


  std::optional<std::string> HuenicornCore::currentEntertainmentConfigurationId() const
  {
    return m_selector->currentEntertainmentConfigurationId();
  }


  glm::ivec2 HuenicornCore::displayResolution() const
  {
    return m_grabber->displayResolution();
  }


  std::vector<glm::ivec2> HuenicornCore::subsampleResolutionCandidates() const
  {
    return m_grabber->subsampleResolutionCandidates();
  }


  unsigned HuenicornCore::subsampleWidth() const
  {
    return m_config.subsampleWidth();
  }


  unsigned HuenicornCore::refreshRate() const
  {
    return m_config.refreshRate();
  }


  unsigned HuenicornCore::maxRefreshRate() const
  {
    return m_grabber->displayRefreshRate();
  }


  Interpolation::Type HuenicornCore::interpolation() const
  {
    return m_config.interpolation();
  }


  const Interpolation::Interpolations& HuenicornCore::availableInterpolations() const
  {
    return Interpolation::availableInterpolations;
  }


  nlohmann::json HuenicornCore::autodetectedBridge() const
  {
    auto detectedBridgeResponse = HttpRequestUtils::sendRequest("https://discovery.meethue.com/", "GET");

    if(!detectedBridgeResponse.has_value()){
      return {{"succeeded", false}, {"error", "Could not reach discovery service. Please check your internet connection."}};
    }

    auto bridges = detectedBridgeResponse.value().asJson();

    return {{"succeeded", true}, {"bridges", bridges}};
  }


  nlohmann::json HuenicornCore::registerNewUser()
  {
    std::string sessionUsername = platformAdapter.getUsername();
    std::string deviceType = "huenicorn#" + sessionUsername;

    nlohmann::json request = {{"devicetype", deviceType}, {"generateclientkey", true}};
    auto response = HttpRequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

    if(!response.has_value()){
      return {{"succeeded", false}, {"error", "unreachable bridge"}};
    }

    auto jsonResponse = response.value().asJson();

    if(jsonResponse.at(0).contains("error")){
      return {{"succeeded", false}, {"error", jsonResponse.at(0).at("error").at("description")}};
    }

    auto credentials = jsonResponse.at(0).at("success").get<Credentials>();
    m_config.setCredentials(credentials);

    return {{"succeeded", true}, {"username", credentials.username()}, {"clientkey", credentials.clientkey()}};
  }


  bool HuenicornCore::setEntertainmentConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(!m_selector->selectEntertainmentConfiguration(entertainmentConfigurationId)){
      return false;
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    return true;
  }


  const UVs& HuenicornCore::setChannelUV(uint8_t channelId, UV&& uv, UVCorner uvCorner)
  {
    return m_channels.at(channelId).setUV(std::move(uv), uvCorner);
  }


  bool HuenicornCore::setChannelGammaFactor(uint8_t channelId, float gammaExponent)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setGammaFactor(gammaExponent);
    return true;
  }


  void HuenicornCore::setSubsampleWidth(unsigned subsampleWidth)
  {
    m_config.setSubsampleWidth(subsampleWidth);
  }


  void HuenicornCore::setRefreshRate(unsigned refreshRate)
  {
    refreshRate = std::min(refreshRate, m_grabber->displayRefreshRate());

    m_config.setRefreshRate(refreshRate);
    refreshRate = m_config.refreshRate();
    m_tickSynchronizer->setTickInterval(1.0f / refreshRate);
  }


  void HuenicornCore::setInterpolation(unsigned interpolation)
  {
    m_config.setInterpolation(static_cast<Interpolation::Type>(interpolation));
  }


  void HuenicornCore::start()
  {
    if(!m_config.initialSetupOk()){
      if(!_runInitialSetup()){
        return;
      }
    }

    if(!_initGrabber()){
      Logger::error("Could not start any grabber");
      return;
    }

    if(!_initSettings()){
      Logger::error("Could not load suitable entertainment configuration.");
      return;

      // TODO : Add tool to create entertainment configurations inside Huenicorn
      // so the official application would no longer be a requirement
    }

    _initWebUI();


    // Spawn UI if no profiles are found
    auto optJsonProfile = _getProfile();
    if(!optJsonProfile.has_value() && !m_openedSetup){
      std::thread spawnBrowser([this](){_spawnBrowser();});
      spawnBrowser.detach();
    }

    _startStreamingLoop();
  }


  void HuenicornCore::stop()
  {
    m_keepLooping = false;
  }


  bool HuenicornCore::validateBridgeAddress(const std::string& bridgeAddress)
  {
    std::string sanitizedAddress = bridgeAddress;

    while(sanitizedAddress.back() == '/'){
      sanitizedAddress.pop_back();
    }

    auto response = HttpRequestUtils::sendRequest(sanitizedAddress + "/api", "GET", "");
    if(!response.has_value()){
      return false;
    }

    m_config.setBridgeAddress(sanitizedAddress);

    return true;
  }


  bool HuenicornCore::validateCredentials(const Credentials& credentials)
  {
    auto response = HttpRequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api/" + credentials.username(), "GET", "");
    if(!response.has_value()){
      return false;
    }

    try{
      auto jsonResponse = response.value().asJson();

      if(jsonResponse.is_array() && jsonResponse.at(0).contains("error")){
        return false;
      }
    }
    catch(const std::exception& e){
      Logger::error(e.what());
      return false;
    }

    m_config.setCredentials(credentials);
    Logger::log("Successfully registered credentials");
    return true;
  }


  bool HuenicornCore::setChannelActivity(uint8_t channelId, bool active)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setActive(active);

    _updateStreamChannelsSize();

    return true;
  }


  void HuenicornCore::saveProfile()
  {
    if(!m_selector->validSelection()){
      Logger::error("There is currently no valid entertainment configuration selected.");
      return;
    }

    nlohmann::json profile;
    if(m_selector->validSelection()){
      profile = nlohmann::json{
        {"entertainmentConfigurationId", m_selector->currentEntertainmentConfigurationId().value()},
        {"channels", nlohmann::json(m_channels)}
      };
    }

    if(!m_config.profileName().has_value()){
      m_config.setProfileName("profile");
    }

    std::ofstream profileFile(_profilePath(), std::ofstream::out);
    profileFile << profile.dump(2) << "\n";
    profileFile.close();
  }


  std::filesystem::path HuenicornCore::_profilePath() const
  {
    if(!m_config.profileName().has_value()){
      return {};
    }

    return m_configRoot / std::filesystem::path(m_config.profileName().value()).replace_extension("json");
  }


  std::optional<nlohmann::json> HuenicornCore::_getProfile()
  {
    std::filesystem::path profilePath = _profilePath();
    nlohmann::json jsonProfile = nlohmann::json::object();

    if(!profilePath.empty() && std::filesystem::exists(profilePath) && std::filesystem::is_regular_file(profilePath)){
      std::ifstream profileFile(profilePath);
      return nlohmann::json::parse(profileFile);
    }

    return std::nullopt;
  }


  bool HuenicornCore::_initSettings()
  {
    if(m_config.refreshRate() == 0){
      m_config.setRefreshRate(m_grabber->displayRefreshRate());
    }

    if(m_config.subsampleWidth() == 0){
      auto displayResolution = m_grabber->displayResolution();
      float percentThreshold = 1.0;
      auto subsampleCandidates = m_grabber->subsampleResolutionCandidates();

      unsigned bestSubsampleWidth = subsampleCandidates.back().x;
      for(int i = subsampleCandidates.size(); i--;){
        unsigned candidate = subsampleCandidates.at(i).x;

        if((static_cast<float>(candidate) / displayResolution.x) * 100 >= percentThreshold){
          bestSubsampleWidth = candidate;
          break;
        }
      }

      m_config.setSubsampleWidth(bestSubsampleWidth);
    }

    const float warningThreshold = 50.0;

    float ratio = static_cast<float>(m_config.subsampleWidth()) / m_grabber->displayResolution().x;

    if(ratio >= warningThreshold / 100){
      Logger::warn("Subsample width is >= ", warningThreshold, "% of the display resolution. Color computation might be intensive.");
    }

    Logger::log("Configuration is ready. Feel free to modify it manually by editing ", std::quoted(m_config.configFilePath().string()));

    const Credentials& credentials = m_config.credentials().value();
    const std::string& bridgeAddress =  m_config.bridgeAddress().value();

    m_selector = std::make_unique<EntertainmentConfigurationSelector>(credentials, bridgeAddress);

    std::string entertainmentConfigurationId = {};
    auto optJsonProfile = _getProfile();

    if(optJsonProfile.has_value()){
      const auto& jsonProfile = optJsonProfile.value();
      if(jsonProfile.contains("entertainmentConfigurationId")){
        entertainmentConfigurationId = jsonProfile.at("entertainmentConfigurationId");
      }
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    return true;
  }


  bool HuenicornCore::_runInitialSetup()
  {
    Logger::log("Starting setup backend");
    unsigned port = m_config.restServerPort();
    const std::string& boundBackendIP = m_config.boundBackendIP();

    SetupBackend sb(this);
    sb.start(port, boundBackendIP);

    if(sb.aborted()){
      Logger::log("Initial setup was aborted");
      return false;
    }

    Logger::log("Finished setup");
    m_openedSetup = true;

    return true;
  }


  bool HuenicornCore::_initGrabber()
  {
    try{
      m_grabber = platformAdapter.getGrabber(&m_config);

      if(m_grabber){
        return true;
      }

      // Falling back on dummy grabber
      if(!m_grabber){
        m_grabber = std::make_unique<DummyGrabber>(&m_config);
        Logger::log("Started DummyGrabber.");
        return true;
      }
    }
    catch(const std::exception& e){
      Logger::error(e.what());
      return false;
    }

    if(!m_grabber){
      Logger::error("Could not find a compatible grabber for your graphic session.");
    }

    return false;
  }


  void HuenicornCore::_initWebUI()
  {
    std::promise<bool> readyWebUIPromise;
    auto readyWebUIFuture = readyWebUIPromise.get_future();

    unsigned restServerPort = m_config.restServerPort();
    const std::string& boundBackendIP = m_config.boundBackendIP();
    m_webUIService.server = std::make_unique<WebUIBackend>(this);
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(restServerPort, boundBackendIP, std::move(readyWebUIPromise));
    });

    readyWebUIFuture.wait();
  }


  void HuenicornCore::_initChannels(const nlohmann::json& jsonChannels)
  {
    const std::string& username = m_config.credentials().value().username();
    const std::string& bridgeAddress =  m_config.bridgeAddress().value();
    Devices devices = ApiTools::loadDevices(username, bridgeAddress);
    EntertainmentConfigurationsChannels entertainmentConfigurationsChannels = ApiTools::loadEntertainmentConfigurationsChannels(username, bridgeAddress);

    Channels channels;

    for(const auto& [id, channel] : m_selector->currentEntertainmentConfiguration().channels){
      bool found = false;
      const auto& members = ApiTools::matchDevices(entertainmentConfigurationsChannels.at(m_selector->currentEntertainmentConfigurationId().value()).at(id), devices);
      for(const auto& jsonProfileChannel : jsonChannels){
        if(jsonProfileChannel.at("channelId") == id){
          bool active = jsonProfileChannel.at("active");
          nlohmann::json jsonUVs = jsonProfileChannel.at("uvs");
          float uvAx = jsonUVs.at("uvA").at("x");
          float uvAy = jsonUVs.at("uvA").at("y");
          float uvBx = jsonUVs.at("uvB").at("x");
          float uvBy = jsonUVs.at("uvB").at("y");

          UVs uvs = {{uvAx, uvAy}, {uvBx, uvBy}};
          float gammaFactor = jsonProfileChannel.at("gammaFactor");
          channels.emplace(id, Channel{active, members, gammaFactor, uvs});

          found = true;
          break;
        }
      }

      if(!found){
        channels.emplace(id, Channel{false, members, 0.0f});
      }
    }

    m_channels = std::move(channels);
    _updateStreamChannelsSize();
  }


  void HuenicornCore::_spawnBrowser()
  {
    while (!m_webUIService.server->running()){
      std::this_thread::sleep_for(100ms);
    }

    std::stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_config.restServerPort();
    std::string serviceURL = serviceUrlStream.str();
    Logger::log("Management WebUI is ready and available at ", serviceURL);

    platformAdapter.openWebBrowser(serviceURL);
  }


  void HuenicornCore::_enableEntertainmentConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(!m_selector->selectEntertainmentConfiguration(entertainmentConfigurationId)){
      return;
    }

    const Credentials& credentials = m_config.credentials().value();

    {
      std::lock_guard lock(m_streamerMutex);
      m_streamer = std::make_unique<Streamer>(credentials, m_config.bridgeAddress().value());
      m_streamer->setEntertainmentConfigurationId(m_selector->currentEntertainmentConfigurationId().value());
    }

    auto profilePath = _profilePath();
    nlohmann::json jsonChannels = nlohmann::json::object();

    if(std::filesystem::is_regular_file(profilePath)){
      nlohmann::json jsonProfile = nlohmann::json::parse(std::ifstream(_profilePath()));
      if(jsonProfile.contains("channels")){
        jsonChannels = jsonProfile.at("channels");
      }
    }

    _initChannels(jsonChannels);
  }


  void HuenicornCore::_startStreamingLoop()
  {
    m_tickSynchronizer = std::make_unique<TickSynchronizer>(1.0f / static_cast<float>(m_config.refreshRate()));

    m_tickSynchronizer->start();

    m_keepLooping = true;
    while(m_keepLooping){
      _update();

      if(!m_tickSynchronizer->sync()){
        const auto& lastExcess = m_tickSynchronizer->lastExcess();
        float percentage = lastExcess.rate * 100;
        Logger::warn("Scheduled interval has been exceeded of ", lastExcess.extra.count(), " (", percentage, "%).\n Please reduce refresh rate if this warning persists.");
      }
    }

    _shutdown();

    m_webUIService.server->stop();
    m_webUIService.thread.value().join();
  }


  void HuenicornCore::_update()
  {
    m_grabber->grabFrameSubsample(m_cvImage);
    if(!m_cvImage.data){
      // Grabbers with asynchronous capture may produce slower than the core consumes
      return;
    }

    cv::Mat subframeImage;

    for(auto& [channelId, channel] : m_channels){
      if(channel.state() == Channel::State::Inactive){
        continue;
      }

      auto& channelStream = m_channelStreams.at(channelId);

      if(channel.state() == Channel::State::PendingShutdown){
        channelStream.id = channelId;
        channelStream.r = 0;
        channelStream.g = 0;
        channelStream.b = 0;
        channel.acknowledgeShutdown();
      }
      else{
        const auto& uvs = channel.uvs();

        glm::ivec2 a{uvs.min.x * m_cvImage.cols, uvs.min.y * m_cvImage.rows};
        glm::ivec2 b{uvs.max.x * m_cvImage.cols, uvs.max.y * m_cvImage.rows};

        ImageProcessing::getSubImage(m_cvImage, a, b).copyTo(subframeImage);
        Color color = ImageProcessing::getDominantColor(subframeImage);

        glm::vec3 normalized = color.toNormalized();
        glm::vec3 correctedColor = glm::pow(normalized, glm::vec3(channel.gammaExponent()));

        channelStream.id = channelId;
        channelStream.r = correctedColor.r;
        channelStream.g = correctedColor.g;
        channelStream.b = correctedColor.b;
      }
    }

    {
      std::lock_guard lock(m_streamerMutex);
      if(m_streamer.get()){
        m_streamer->streamChannels(m_channelStreams);
      }
    }
  }


  void HuenicornCore::_shutdown()
  {
    m_selector->disableStreaming();
  }


  void HuenicornCore::_updateStreamChannelsSize()
  {
    m_channelStreams.resize(m_channels.size());
  }
}
