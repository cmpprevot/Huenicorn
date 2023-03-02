#include <Huenicorn/Streamer.hpp>

#include <iostream>

#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>

#include <Huenicorn/RequestUtils.hpp>


using namespace glm;
using namespace nlohmann;
using namespace std;


namespace Huenicorn
{
  void Streamer::_setStreamActive(const EntertainmentConfig& entertainmentConfig, bool active)
  {
    json jsonBody = {
      {"action", active ? "start" : "stop"},
      {"metadata", {{"name", entertainmentConfig.name()}}}
    };

    RequestUtils::Headers headers = {
      {"hue-application-key", m_username}
    };


    string url = "https://" + m_address + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfig.id();

    auto r = RequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);
  }


  Streamer::Streamer(const std::string& username, const std::string& clientkey, const std::string& address, const std::string& port):
    m_username(username),
    m_clientkey(clientkey),
    m_address(address),
    m_dtlsClient(m_username, m_clientkey, m_address, port)
  {
    _loadEntertainmentData();
    _selectEntertainementConfig();

    _setStreamActive(m_selectedConfig.value(), false);
    _setStreamActive(m_selectedConfig.value(), true);
  }


  Streamer::~Streamer()
  {
  }


  bool Streamer::start()
  {
    try{
      m_dtlsClient.init();
    }
    catch(const std::exception& e){
      cout << e.what();
      return false;
    }

    m_keepStreaming = true;

    m_streamThread.emplace([this](){this->_streamingLoop();});
    m_streamThread.value().join();


    return m_keepStreaming;
  }


  void Streamer::stop()
  {
    m_keepStreaming = false;
    _setStreamActive(m_selectedConfig.value(), false);
  }


  void Streamer::_loadEntertainmentData()
  {
    RequestUtils::Headers headers = {
      {"hue-application-key", m_username}
    };

    string entertainmentConfUrl = "https://" + m_address + "/clip/v2/resource/entertainment_configuration";


    auto entertainmentConfResponse = RequestUtils::sendRequest(entertainmentConfUrl, "GET", "", headers);

    if(entertainmentConfResponse.at("errors").size() == 0){
      // Listing entertainment configurations
      for(const json& jsonEntertainentConfiguration : entertainmentConfResponse.at("data")){
        string confId = jsonEntertainentConfiguration.at("id");
        string confName = jsonEntertainentConfiguration.at("metadata").at("name");


        const json& lightServices = jsonEntertainentConfiguration.at("light_services");

        unordered_map<string, Light> lights;
        
        for(const json& lightService : lightServices){
          const string& lightId = lightService.at("rid");
          string lightUrl = "https://" + m_address + "/clip/v2/resource/light/" + lightId;

          auto jsonLightData = RequestUtils::sendRequest(lightUrl, "GET", "", headers);
          const json& metadata = jsonLightData.at("data").at(0).at("metadata");

          lights.insert({lightId, {lightId, metadata.at("name"), metadata.at("archetype")}});
        }

        vector<uint8_t> channelIds;
        for(const auto& channel : jsonEntertainentConfiguration.at("channels")){
          channelIds.push_back(channel.at("channel_id"));
        }

        m_entertainmentConfigs.emplace_back(confId, confName, lights, channelIds);
      }
    }
  }


  void Streamer::_selectEntertainementConfig()
  {
    // Todo : proper setter
    m_selectedConfig.emplace(m_entertainmentConfigs.front());
  }


  const std::string& Streamer::_entertainmentId()
  {
    return m_selectedConfig.value().id();
  }


  void Streamer::_streamingLoop()
  {
    m_header.setEntertainmentConfigurationId(_entertainmentId());

    float timeFactor = 1.f;
    float pi = glm::pi<float>();
    float thirdOfPi = pi / 3;

    while(m_keepStreaming){
      if(m_selectedConfig.has_value()){
        vector<char> requestBuffer;
        float coeff = (m_header.sequenceId / static_cast<float>(numeric_limits<uint8_t>::max()));
        float angle = timeFactor * coeff * 2 * pi;

        float rIntensity = 0.5 + 0.5 * glm::sin(angle);
        float gIntensity = 0.5 + 0.5 * glm::sin(angle + thirdOfPi);
        float bIntensity = 0.5 + 0.5 * glm::sin(angle - thirdOfPi);

        requestBuffer.insert(requestBuffer.end(), (char*)&m_header, (char*)&m_header + sizeof(HuestreamHeader));


        const auto& channelIds = m_selectedConfig.value().channelIds();

        vector<HuestreamPayload> payloads(channelIds.size());
        for(const auto channel : channelIds){
          HuestreamPayload payload;
          payload.channelId = channel;
          payload.setR(static_cast<uint16_t>(0xffff * rIntensity));
          payload.setG(static_cast<uint16_t>(0xffff * gIntensity));
          payload.setB(static_cast<uint16_t>(0xffff * bIntensity));

          requestBuffer.insert(requestBuffer.end(), (char*)&payload, (char*)&payload + sizeof(HuestreamPayload));
        }

        m_dtlsClient.send(requestBuffer);

        m_header.sequenceId++;
      }

      this_thread::sleep_for(20ms);
    }
  }
}
