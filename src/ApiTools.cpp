#include <Huenicorn/ApiTools.hpp>

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

#include <Huenicorn/RequestUtils.hpp>

using namespace nlohmann;
using namespace std;

namespace Huenicorn
{
  namespace ApiTools
  {
    EntertainmentConfigs loadEntertainmentConfigurations(const string& username, const string& address)
    {
      EntertainmentConfigs entertainmentConfigs;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string entertainmentConfUrl = "https://" + address + "/clip/v2/resource/entertainment_configuration";
      auto entertainmentConfResponse = RequestUtils::sendRequest(entertainmentConfUrl, "GET", "", headers);

      if(entertainmentConfResponse.at("errors").size() == 0){
        // Listing entertainment configurations
        for(const json& jsonEntertainentConfiguration : entertainmentConfResponse.at("data")){
          string confId = jsonEntertainentConfiguration.at("id");
          string confName = jsonEntertainentConfiguration.at("metadata").at("name");

          const json& lightServices = jsonEntertainentConfiguration.at("light_services");

          unordered_map<string, Device> lights;
          
          for(const json& lightService : lightServices){
            const string& lightId = lightService.at("rid");

            string lightUrl = "https://" + address + "/clip/v2/resource/light/" + lightId;
            auto jsonLightData = RequestUtils::sendRequest(lightUrl, "GET", "", headers);
            const json& metadata = jsonLightData.at("data").at(0).at("metadata");

            lights.insert({lightId, {lightId, metadata.at("name"), metadata.at("archetype")}});
          }

          Channels channels;
          for(const auto& channel : jsonEntertainentConfiguration.at("channels")){
            channels.insert({channel.at("channel_id").get<uint8_t>(), {}});
          }

          entertainmentConfigs.insert({confId, {confName, lights, channels}});
        }
      }

      return entertainmentConfigs;
    }


    Devices loadDevices(const string& username, const string& address)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string resourceUrl = "https://" + address + "/clip/v2/resource";
      auto jsonResource = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      std::cout << resourceUrl << std::endl;

      Devices devices;

      for(const auto& jsonData : jsonResource.at("data")){
        if(jsonData.at("type") == "device"){
          const auto& jsonServices = jsonData.at("services");

          for(const auto& service : jsonServices){
            if(service.at("rtype") == "entertainment"){
              string deviceId = service.at("rid");
              string name = jsonData.at("metadata").at("name");
              string archetype = jsonData.at("metadata").at("archetype");

              devices.emplace(deviceId, Device{deviceId, name, archetype});
            }
          }
        }
      }
      
      return devices;
    }


    ConfigurationsChannels loadConfigurationsChannels(const string& username, const string& address)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string resourceUrl = "https://" + address + "/clip/v2/resource/entertainment_configuration";

      auto jsonEntertainmentConfigurations = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      ConfigurationsChannels configurationsChannels;

      for(const auto& entConf : jsonEntertainmentConfigurations.at("data")){
        string configurationId = entConf.at("id");
        for(const auto& jsonChannel : entConf.at("channels")){
          uint8_t channelId = jsonChannel.at("channel_id");
          for(const auto& jsonMember : jsonChannel.at("members")){
            string jsonMemberId = jsonMember.at("service").at("rid");
            configurationsChannels[configurationId][channelId].push_back(jsonMemberId);
          }
        }
      }

      return configurationsChannels;
    }


    vector<Device> channelDevices(const MembersIds& channel, const Devices& devices)
    {
      vector<Device> channelDevices;
      for(const auto& memberId : channel){
        const auto& it = devices.find(memberId);
        channelDevices.push_back(it->second);
      }

      return channelDevices;
    }


    void setSelectedConfigStreamActivity(bool active, const EntertainmentConfigEntry& entertrainmentConfigurationEntry, const string& username, const string& address)
    {
      json jsonBody = {
        {"action", active ? "start" : "stop"},
        {"metadata", {{"name", entertrainmentConfigurationEntry.second.name()}}}
      };

      RequestUtils::Headers headers = {{"hue-application-key", username}};

      string url = "https://" + address + "/clip/v2/resource/entertainment_configuration/" + entertrainmentConfigurationEntry.first;

      RequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);
    }


    bool streamingActive(const EntertainmentConfigEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& address)
    {
      string status;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string url = "https://" + address + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;
      auto response = RequestUtils::sendRequest(url, "GET", "", headers);

      if(response.at("errors").size() == 0){
        status = response.at("data").front().at("status");
      }

      return status == "active";
    }
  }
}
