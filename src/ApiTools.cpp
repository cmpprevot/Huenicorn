#include <Huenicorn/ApiTools.hpp>

#include <nlohmann/json.hpp>

#include <Huenicorn/RequestUtils.hpp>


using namespace nlohmann;
using namespace std;

namespace Huenicorn
{
  namespace ApiTools
  {
    EntertainmentConfigurations loadEntertainmentConfigurations(const string& username, const string& bridgeAddress)
    {
      EntertainmentConfigurations entertainmentConfigurations;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string entertainmentConfUrl = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration";
      auto entertainmentConfigurationResponse = RequestUtils::sendRequest(entertainmentConfUrl, "GET", "", headers);

      if(entertainmentConfigurationResponse.at("errors").size() == 0){
        // Listing entertainment configurations
        for(const json& jsonEntertainentConfiguration : entertainmentConfigurationResponse.at("data")){
          string confId = jsonEntertainentConfiguration.at("id");
          string confName = jsonEntertainentConfiguration.at("metadata").at("name");

          const json& lightServices = jsonEntertainentConfiguration.at("light_services");

          unordered_map<string, Device> lights;
          
          for(const json& lightService : lightServices){
            const string& lightId = lightService.at("rid");

            string lightUrl = "https://" + bridgeAddress + "/clip/v2/resource/light/" + lightId;
            auto jsonLightData = RequestUtils::sendRequest(lightUrl, "GET", "", headers);
            const json& metadata = jsonLightData.at("data").at(0).at("metadata");

            lights.insert({lightId, {lightId, metadata.at("name"), metadata.at("archetype")}});
          }

          Channels channels;
          for(const auto& jsonChannel : jsonEntertainentConfiguration.at("channels")){
            channels.insert({jsonChannel.at("channel_id").get<uint8_t>(), {false, {}, 0.f}});
          }

          entertainmentConfigurations.insert({confId, {confName, lights, channels}});
        }
      }

      return entertainmentConfigurations;
    }


    Devices loadDevices(const string& username, const string& bridgeAddress)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string resourceUrl = "https://" + bridgeAddress + "/clip/v2/resource";
      auto jsonResource = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

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


    EntertainmentConfigurationsChannels loadEntertainmentConfigurationsChannels(const string& username, const string& bridgeAddress)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string resourceUrl = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration";

      auto jsonEntertainmentConfigurations = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      EntertainmentConfigurationsChannels entertainmentConfigurationsChannels;

      for(const auto& entConf : jsonEntertainmentConfigurations.at("data")){
        string configurationId = entConf.at("id");
        for(const auto& jsonChannel : entConf.at("channels")){
          uint8_t channelId = jsonChannel.at("channel_id");
          for(const auto& jsonMember : jsonChannel.at("members")){
            string jsonMemberId = jsonMember.at("service").at("rid");
            entertainmentConfigurationsChannels[configurationId][channelId].push_back(jsonMemberId);
          }
        }
      }

      return entertainmentConfigurationsChannels;
    }


    vector<Device> matchDevices(const MembersIds& membersIds, const Devices& devices)
    {
      vector<Device> matchedDevices;
      for(const auto& memberId : membersIds){
        const auto& it = devices.find(memberId);
        matchedDevices.push_back(it->second);
      }

      return matchedDevices;
    }


    void setStreamingState(const EntertainmentConfigurationEntry& entertrainmentConfigurationEntry, const string& username, const string& bridgeAddress, bool active)
    {
      json jsonBody = {
        {"action", active ? "start" : "stop"},
        {"metadata", {{"name", entertrainmentConfigurationEntry.second.name()}}}
      };

      RequestUtils::Headers headers = {{"hue-application-key", username}};

      string url = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertrainmentConfigurationEntry.first;

      RequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);
    }


    bool streamingActive(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress)
    {
      string status;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string url = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;
      auto response = RequestUtils::sendRequest(url, "GET", "", headers);
      if(response.at("errors").size() == 0){
        status = response.at("data").front().at("status");
      }

      return status == "active";
    }
  }
}
