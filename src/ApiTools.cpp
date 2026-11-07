#include <Huenicorn/ApiTools.hpp>

#include <algorithm>

#include <nlohmann/json.hpp>

#include <Huenicorn/BridgeAddress.hpp>
#include <Huenicorn/Color.hpp>
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


    nlohmann::json buildLightStateBody(const nlohmann::json& state)
    {
      nlohmann::json body = nlohmann::json::object();

      bool on = state.value("on", true);
      body["on"] = {{"on", on}};

      if(on){
        if(state.contains("brightness")){
          double brightness = std::clamp(state.at("brightness").get<double>(), 0.0, 100.0);
          body["dimming"] = {{"brightness", brightness}};
        }

        std::string mode = state.value("mode", "color");
        if(mode == "white" && state.contains("mirek")){
          int mirek = std::clamp(state.at("mirek").get<int>(), 153, 500);
          body["color_temperature"] = {{"mirek", mirek}};
        }
        else if(state.contains("r") && state.contains("g") && state.contains("b")){
          Color color(
            static_cast<Color::ChannelDepth>(std::clamp(state.at("r").get<int>(), 0, 255)),
            static_cast<Color::ChannelDepth>(std::clamp(state.at("g").get<int>(), 0, 255)),
            static_cast<Color::ChannelDepth>(std::clamp(state.at("b").get<int>(), 0, 255))
          );
          // Hue "gamut C" boundaries; Color::toXY currently does not clamp against them
          static const Color::GamutCoordinates gamutC = {{{0.675f, 0.322f}, {0.409f, 0.518f}, {0.167f, 0.04f}}};
          glm::vec2 xy = color.toXY(gamutC);
          body["color"] = {{"xy", {{"x", xy.x}, {"y", xy.y}}}};
        }
      }

      return body;
    }


    bool setLightsState(const std::string& username, const std::string& bridgeAddress, const MembersIds& lightIds, const nlohmann::json& state)
    {
      nlohmann::json body;
      try{
        body = buildLightStateBody(state);
      }
      catch(const std::exception& e){
        Logger::warn("setLightsState: invalid state, ignoring it: ", e.what());
        return false;
      }

      HttpRequestUtils::Headers headers = {{"hue-application-key", username}};

      bool allSucceeded = true;
      for(const std::string& lightId : lightIds){
        std::string lightUrl = HttpProtocol + bridgeAddress + "/clip/v2/resource/light/" + lightId;

        auto response = HttpRequestUtils::sendRequest(lightUrl, "PUT", body.dump(), headers);
        if(!response.has_value()){
          Logger::warn("setLightsState on light ", lightId, ": no response from bridge");
          allSucceeded = false;
          continue;
        }

        try{
          // CLIP v2 answers with an array of {"success": ...} or {"error": ...} entries
          for(const nlohmann::json& entry : response.value().asJson()){
            if(entry.contains("error")){
              Logger::warn("setLightsState on light ", lightId, " rejected: ", entry.at("error").dump());
              allSucceeded = false;
            }
          }
        }
        catch(const std::exception& e){
          Logger::warn("setLightsState on light ", lightId, ": unparsable response: ", e.what());
          allSucceeded = false;
        }
      }

      return allSucceeded;
    }
  }
}
