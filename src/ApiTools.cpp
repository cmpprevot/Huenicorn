#include <Huenicorn/ApiTools.hpp>

#include <nlohmann/json.hpp>

#include <Huenicorn/BridgeAddress.hpp>
#include <Huenicorn/HttpRequestUtils.hpp>
#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  namespace ApiTools
  {
    EntertainmentConfigurations loadEntertainmentConfigurations(const std::string& username, const std::string& bridgeAddress)
    {
      // I don't always abbreviate variable names
      // but when I do, it's because I don't have a 32:9 monitor
      // (If someones has such a display, please tell me about Huenicorn's performance)
      EntertainmentConfigurations entConfs;

      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string entConfUrl = HttpProtocol + bridgeAddress + "/clip/v2/resource/entertainment_configuration";
      auto entConfResponse = HttpRequestUtils::sendRequest(entConfUrl, "GET", "", headers);

      if(entConfResponse.has_value()){
        // Listing entertainment configurations

        auto jsonEntConfs = entConfResponse.value().asJson();

        for(const nlohmann::json& jsonEntConf : jsonEntConfs.at("data")){

          EntertainmentConfiguration entConf = jsonEntConf.get<EntertainmentConfiguration>();

          for(auto& [lightId, device] : entConf.devices){
            std::string lightUrl = HttpProtocol + bridgeAddress + "/clip/v2/resource/light/" + lightId;

            auto jsonLightData = HttpRequestUtils::sendRequest(lightUrl, "GET", "", headers).value().asJson();
            auto deviceId = device.id;
            device = jsonLightData.at("data").at(0).at("metadata").get<Device>();
            device.id = deviceId;
          }

          const auto& jsonChannels = jsonEntConf.at("channels");
          for (const auto& jsonChannel : jsonChannels) {
            uint8_t channelId = jsonChannel.at("channel_id").get<uint8_t>();
            entConf.channels.insert({channelId, Channel{false, {}, 0.f}});
          }

          entConfs.insert({jsonEntConf.at("id").get<std::string>(), entConf});
        }
      }

      return entConfs;
    }


    Devices loadDevices(const std::string& username, const std::string& bridgeAddress)
    {
      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string resourceUrl = HttpProtocol + bridgeAddress + "/clip/v2/resource";
      auto resourceResponse = HttpRequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      Devices devices;

      if(resourceResponse.has_value()){
        auto jsonResource = resourceResponse.value().asJson();
        for(const auto& jsonData : jsonResource.at("data")){
          if(jsonData.at("type") == "device"){
            const auto& jsonServices = jsonData.at("services");

            for(const auto& service : jsonServices){
              if(service.at("rtype") == "entertainment"){
                std::string deviceId = service.at("rid");

                auto device = jsonData.at("metadata").get<Device>();
                device.id = deviceId;
                devices.emplace(deviceId, device);
              }
            }
          }
        }
      }

      return devices;
    }


    EntertainmentToLightIds loadEntertainmentToLightIds(const std::string& username, const std::string& bridgeAddress)
    {
      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string resourceUrl = HttpProtocol + bridgeAddress + "/clip/v2/resource/device";
      auto resourceResponse = HttpRequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      EntertainmentToLightIds entToLight;

      if(resourceResponse.has_value()){
        auto jsonResource = resourceResponse.value().asJson();
        for(const auto& jsonDevice : jsonResource.at("data")){
          if(jsonDevice.at("type") != "device"){
            continue;
          }

          std::vector<std::string> lightIds;
          std::string entertainmentId;
          for(const auto& service : jsonDevice.at("services")){
            if(service.at("rtype") == "light"){
              lightIds.push_back(service.at("rid"));
            }
            else if(service.at("rtype") == "entertainment"){
              entertainmentId = service.at("rid");
            }
          }

          if(!entertainmentId.empty() && !lightIds.empty()){
            entToLight[entertainmentId] = std::move(lightIds);
          }
        }
      }

      return entToLight;
    }


    EntertainmentConfigurationsChannels loadEntertainmentConfigurationsChannels(const std::string& username, const std::string& bridgeAddress)
    {
      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string resourceUrl = HttpProtocol + bridgeAddress + "/clip/v2/resource/entertainment_configuration";

      auto entertainmentConfigurationsResponse = HttpRequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      EntertainmentConfigurationsChannels entConfsChannels;

      if(entertainmentConfigurationsResponse.has_value()){
        auto jsonEntertainmentConfigurations = entertainmentConfigurationsResponse.value().asJson();
        for(const auto& entConf : jsonEntertainmentConfigurations.at("data")){
          std::string configurationId = entConf.at("id");
          for(const auto& jsonChannel : entConf.at("channels")){
            uint8_t channelId = jsonChannel.at("channel_id");
            for(const auto& jsonMember : jsonChannel.at("members")){
              std::string jsonMemberId = jsonMember.at("service").at("rid");
              entConfsChannels[configurationId][channelId].push_back(jsonMemberId);
            }
          }
        }
      }

      return entConfsChannels;
    }


    std::vector<Device> matchDevices(const MembersIds& membersIds, const Devices& devices)
    {
      std::vector<Device> matchedDevices;
      for(const auto& memberId : membersIds){
        const auto& it = devices.find(memberId);
        matchedDevices.push_back(it->second);
      }

      return matchedDevices;
    }


    void setStreamingState(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress, bool active)
    {
      nlohmann::json jsonBody = {
        {"action", active ? "start" : "stop"},
        {"metadata", {{"name", entertainmentConfigurationEntry.second.name}}}
      };

      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};

      std::string url = HttpProtocol + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;

      auto response = HttpRequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);

      if(!response.has_value()){
        Logger::warn("setStreamingState(", active ? "start" : "stop", ") on ", entertainmentConfigurationEntry.first, ": no response from bridge");
      }
      else{
        Logger::log("setStreamingState(", active ? "start" : "stop", ") on ", entertainmentConfigurationEntry.first, " response: ", response.value().asString());
      }
    }


    bool streamingActive(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress)
    {
      std::string status;

      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string url = HttpProtocol + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;
      auto entConfResponse = HttpRequestUtils::sendRequest(url, "GET", "", headers);
      if(entConfResponse.has_value()){
        status = entConfResponse.value().asJson().at("data").front().at("status");
      }

      return status == "active";
    }


  }
}
