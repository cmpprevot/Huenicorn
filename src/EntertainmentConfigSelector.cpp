#include <Huenicorn/EntertainmentConfigSelector.hpp>

#include <iostream>

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


  const std::string& EntertainmentConfigSelector::selectedEntertainmentConfigId() const
  {
    return m_selectedConfig->first;
  }


  const EntertainmentConfig& EntertainmentConfigSelector::selectedConfig() const
  {
    return m_selectedConfig->second;
  }

  
  bool EntertainmentConfigSelector::validSelecion() const
  {
    return m_selectedConfig != m_entertainmentConfigs.end();
  }


  bool EntertainmentConfigSelector::selectEntertainementConfig(const std::string& entertainmentConfigId)
  {
    if(m_entertainmentConfigs.size() == 0){
      return false;
    }

    if(entertainmentConfigId == ""){
      cout << "Fallback selection" << endl;
      m_selectedConfig = m_entertainmentConfigs.begin();
    }
    else{
      m_selectedConfig = m_entertainmentConfigs.find(entertainmentConfigId);
      cout << "Selected : " << entertainmentConfigId << endl;

      if(m_selectedConfig == m_entertainmentConfigs.end()){
        cout << "Invalid selection : " << entertainmentConfigId << endl;
        return false;
      }
    }
    
    _setSelectedConfigStreamActivity(true);

    return true;
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

        Channels channels;
        for(const auto& channel : jsonEntertainentConfiguration.at("channels")){
          channels.insert({channel.at("channel_id").get<uint8_t>(), {}});
        }

        m_entertainmentConfigs.insert({confId, {confName, lights, channels}});
      }
    }
  }


  void EntertainmentConfigSelector::_setSelectedConfigStreamActivity(bool active) const
  {
    json jsonBody = {
      {"action", active ? "start" : "stop"},
      {"metadata", {{"name", m_selectedConfig->second.name()}}}
    };

    RequestUtils::Headers headers = {
      {"hue-application-key", m_username}
    };


    string url = "https://" + m_address + "/clip/v2/resource/entertainment_configuration/" + m_selectedConfig->first;

    auto r = RequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);
  }


  void EntertainmentConfigSelector::_clearConfigs()
  {
    m_entertainmentConfigs.clear();
    m_selectedConfig = m_entertainmentConfigs.end();
  }
}
