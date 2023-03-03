#include <Huenicorn/EntertainmentConfigSelector.hpp>

#include <json/json.h>

#include <Huenicorn/RequestUtils.hpp>

using namespace nlohmann;
using namespace std;

namespace Huenicorn
{
  EntertainmentConfigSelector::EntertainmentConfigSelector(const std::string& username, const std::string& clientkey, const std::string& address):
  m_username(username),
  m_clientkey(clientkey),
  m_address(address)
  {
    _loadEntertainmentData();
  }


  const std::string& EntertainmentConfigSelector::entertainmentConfigId() const
  {
    return m_selectedConfig.value().id();
  }


  const EntertainmentConfig& EntertainmentConfigSelector::selectedConfig() const
  {
    return m_selectedConfig.value();
  }


  void EntertainmentConfigSelector::selectEntertainementConfig(const std::string& /*entertainmentConfig*/)
  {
    // Todo : use parameter
    m_selectedConfig.emplace(m_entertainmentConfigs.front());
    _setStreamActive(m_selectedConfig.value(), true);
  }



  void EntertainmentConfigSelector::_loadEntertainmentData()
  {
    _clearConfigs();

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

        vector<Channel> channels;
        for(const auto& channel : jsonEntertainentConfiguration.at("channels")){
          channels.emplace_back(channel.at("channel_id"));
        }

        m_entertainmentConfigs.emplace_back(confId, confName, lights, channels);
      }
    }
  }

  void EntertainmentConfigSelector::_setStreamActive(const EntertainmentConfig& entertainmentConfig, bool active)
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


  void EntertainmentConfigSelector::_clearConfigs()
  {
    m_entertainmentConfigs.clear();
    m_selectedConfig.reset();
  }

}
