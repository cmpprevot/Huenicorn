#include <Huenicorn/HuenicornCore.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

#include <unistd.h>

#include <glm/trigonometric.hpp>

#include <Huenicorn/X11Grabber.hpp>
#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/RequestUtils.hpp>
#include <Huenicorn/SetupBackend.hpp>
#include <Huenicorn/WebUIBackend.hpp>
#include <Huenicorn/JsonCast.hpp>


using namespace nlohmann;
using namespace glm;
using namespace std;


namespace Huenicorn
{
  HuenicornCore::HuenicornCore(const std::filesystem::path& configRoot):
  m_configRoot(configRoot),
  m_profileFilePath(m_configRoot / "profile.json"),
  m_config(m_configRoot),
  m_grabber(make_unique<X11Grabber>(&m_config))
  {}


  const std::filesystem::path HuenicornCore::configFilePath() const
  {
    return m_config.configFilePath();
  }

  const Channels& HuenicornCore::channels() const
  {
    return m_channels;
  }


  glm::ivec2 HuenicornCore::screenResolution() const
  {
    return m_grabber->getScreenResolution();
  }


  vector<glm::ivec2> HuenicornCore::subsampleResolutionCandidates() const
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


  json HuenicornCore::autoDetectedBridge() const
  {
    string bridgeAddress;
    try{
      auto detectedBridgeData = RequestUtils::sendRequest("https://discovery.meethue.com/", "GET");

      if(detectedBridgeData.size() < 1){
        return {{"succeeded", false}, {"error", "Could not autodetect bridge."}};
      }

      bridgeAddress = detectedBridgeData.front().at("internalipaddress");
    }
    catch(const json::exception& e){
      return {{"succeeded", false}, {"error", "Could not autodetect bridge."}};
    }

    return {{"succeeded", bridgeAddress != ""}, {"bridgeAddress", bridgeAddress}};
  }


  json HuenicornCore::registerNewUser()
  {
    string login = getlogin();
    string deviceType = "huenicorn#" + login;

    json request = {{"devicetype", deviceType}, {"generateclientkey", true}};
    auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

    if(response.size() < 1){
      return {{"succeeded", false}, {"error", "unreachable bridge"}};
    }

    if(response.at(0).contains("error")){
      return {{"succeeded", false}, {"error", response.at(0).at("error").at("description")}};
    }

    string username = response.at(0).at("success").at("username");
    string clientkey = response.at(0).at("success").at("clientkey");
    m_config.setUsername(username);
    m_config.setClientkey(clientkey);

    return {{"succeeded", true}, {"username", username}, {"clientkey", clientkey}};
  }


  const UVs& HuenicornCore::setChannelUV(uint8_t channelId, UV&& uv, UVType uvType)
  {
    return m_channels.at(channelId).setUV(std::move(uv), uvType);
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
    m_config.setRefreshRate(refreshRate);
    refreshRate = m_config.refreshRate();
    m_tickSynchronizer->setTickInterval(1.0f / refreshRate);
  }


  void HuenicornCore::start()
  {
    unsigned port = m_config.restServerPort();

    if(!m_config.initialSetupOk()){
      cout << "Starting setup backend" << endl;

      SetupBackend sb(this);
      sb.start(port);

      if(sb.aborted()){
        cout << "Initial setup was aborted" << endl;
        return;
      }

      cout << "Finished setup" << endl;
      m_openedSetup = true;
    }

    if(!m_config.initialSetupOk()){
      cout << "There are errors in the config file" << endl;
      return;
    }

    if(m_config.subsampleWidth() == 0){
      m_config.setSubsampleWidth(m_grabber->subsampleResolutionCandidates().back().x);
    }

    cout << "Configuration is ready. Feel free to modify it manually by editing " << std::quoted(m_config.configFilePath().string()) << endl;

    _loop();
  }


  void HuenicornCore::stop()
  {
    m_keepLooping = false;
  }


  bool HuenicornCore::validateBridgeAddress(const std::string& bridgeAddress)
  {
    try{
      string sanitizedAddress = bridgeAddress;
      while (sanitizedAddress.back() == '/'){
        sanitizedAddress.pop_back();
      }

      auto response = RequestUtils::sendRequest(sanitizedAddress + "/api", "GET", "");
      if(response.size() == 0){
        return false;
      }

      m_config.setBridgeAddress(sanitizedAddress);
    }
    catch(const json::exception& exception){
      cout << exception.what() << endl;
      return false;
    }

    return true;
  }


  bool HuenicornCore::validateCredentials(const std::string& username, const std::string& clientkey)
  {
    try{
      auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api/" + username, "GET", "");
      if(response.size() == 0){
        return false;
      }

      if(response.is_array() && response.at(0).contains("error")){
        return false;
      }

      m_config.setUsername(username);
      m_config.setClientkey(clientkey);

      cout << "Successfully registered credentials" << endl;
    }
    catch(const json::exception& exception){
      cout << exception.what() << endl;
      return false;
    }

    return true;
  }


  bool HuenicornCore::setChannelActivity(uint8_t channelId, bool active)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setActive(active);
    return true;
  }


  void HuenicornCore::saveProfile() const
  {
    if(!m_selector->validSelecion()){
      cout << "There is currently no valid entertainment configuration selected." << endl;
      return;
    }

    nlohmann::json profile = json{
      {"entertainmentConfigId", m_selector->selectedEntertainmentConfigId()},
      {"channels", JsonCast::serialize(m_channels)}
    };

    ofstream profileFile(m_profileFilePath, ofstream::out);
    profileFile << profile.dump(2) << endl;
    profileFile.close();
  }


  bool HuenicornCore::_loadProfile()
  {
    filesystem::path profilePath = m_profileFilePath;

    if(!filesystem::exists(profilePath) || !filesystem::is_regular_file(profilePath)){
      cout << "No profile found yet." << endl;
      m_selector->selectEntertainementConfig("");
      return false;
    }

    ifstream profileFile(profilePath);
    json jsonProfile = json::parse(profileFile);

    string entertainmentConfigId = "";
    if(jsonProfile.contains("entertainmentConfigId")){
      m_selector->selectEntertainementConfig(jsonProfile.at("entertainmentConfigId")); // ToDo : Proper selection
    }

    if(!m_selector->selectEntertainementConfig(entertainmentConfigId)){
      cout << "Invalid entertaintment config selection" << endl;
      return false;
    }

    for(const auto& [id, channel] : m_selector->selectedConfig().channels()){
      bool found = false;
      for(const auto& jsonProfileChannel : jsonProfile.at("channels")){
        if(jsonProfileChannel.at("channelId") == id){
          bool active = jsonProfileChannel.at("active");
          json jsonUVs = jsonProfileChannel.at("uvs");
          float uvAx = jsonUVs.at("uvA").at("x");
          float uvAy = jsonUVs.at("uvA").at("y");
          float uvBx = jsonUVs.at("uvB").at("x");
          float uvBy = jsonUVs.at("uvB").at("y");

          UVs uvs = {{uvAx, uvAy}, {uvBx, uvBy}};
          float gammaFactor = jsonProfileChannel.at("gammaFactor");
          m_channels.insert({id, {active, uvs, gammaFactor}});
          found = true;
          break;
        }
      }

      if(!found){
        m_channels.insert({id, {false}});
      }
    }

    return true;
  }


  void HuenicornCore::_spawnBrowser()
  {
    while (!m_webUIService.server->running()){
      this_thread::sleep_for(100ms);
    }
    
    stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_config.restServerPort();
    string serviceURL = serviceUrlStream.str();
    std::cout << "Management WebUI is ready and available at " << serviceURL << std::endl;

    system(string("xdg-open " + serviceURL).c_str());
  }


  void HuenicornCore::_loop()
  {
    unsigned restServerPort = m_config.restServerPort();
    m_webUIService.server = make_unique<WebUIBackend>(this);
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(restServerPort);
    });

    const string& username = m_config.username().value();
    const string& clientkey = m_config.clientkey().value();
    const string& bridgeAddress =  m_config.bridgeAddress().value();
    m_selector = make_unique<EntertainmentConfigSelector>(username, clientkey, bridgeAddress);

    if(!_loadProfile() && ! m_openedSetup){
      thread spawnBrowser([this](){_spawnBrowser();});
      spawnBrowser.detach();
    }

    if(!m_selector->validSelecion()){
      cout << "No entertainment configuration was found" << endl;
      return;
    }

    m_streamer = make_unique<Streamer>(m_config.username().value(), m_config.clientkey().value(), m_config.bridgeAddress().value());

    m_streamer->setEntertainmentConfigId(m_selector->selectedEntertainmentConfigId());
    // Todo : Check if handshake went well before proceding

    m_tickSynchronizer = make_unique<TickSynchronizer>(1.0f / static_cast<float>(m_config.refreshRate()));

    m_tickSynchronizer->start();

    m_keepLooping = true;
    while(m_keepLooping){
      _processScreenFrame();

      if(!m_tickSynchronizer->sync()){
        cout << "Scheduled interval has been exceeded of " << m_tickSynchronizer->lastExcess().extra << " (" << m_tickSynchronizer->lastExcess().rate * 100 << "%)." << endl;
        cout << "Please reduce refreshRate if this warning persists." << endl;
      }
    }

    _shutdownLights();

    m_webUIService.server->stop();
    m_webUIService.thread.value().join();
  }


  void HuenicornCore::_processScreenFrame()
  {
    m_grabber->getScreenSubsample(m_cvImage);
    cv::Mat subImage;
    vector<ChannelStream> channelStreams;

    for(const auto& [channelId, channel] : m_channels){
      const auto& uvs = channel.uvs();

      glm::ivec2 a{uvs.min.x * m_cvImage.cols, uvs.min.y * m_cvImage.rows};
      glm::ivec2 b{uvs.max.x * m_cvImage.cols, uvs.max.y * m_cvImage.rows};

      ImageProcessing::getSubImage(m_cvImage, a, b).copyTo(subImage);
      Color color = ImageProcessing::getDominantColors(subImage, 1).front();

      glm::vec3 normalized = color.toNormalized();
      channelStreams.push_back({channelId, normalized.r, normalized.g, normalized.b});
    }

    m_streamer->streamChannels(channelStreams);
  }


  void HuenicornCore::_shutdownLights()
  {
    /*
    for(const auto& [_, syncedLight] : m_syncedLights){
      syncedLight->setState(false);
    }
    */
  }
}
