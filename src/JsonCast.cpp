#include <Huenicorn/JsonCast.hpp>

using namespace nlohmann;

namespace Huenicorn
{
  namespace JsonCast
  {
    nlohmann::json serialize(const UVs& uvs)
    {
      return {
        {"uvA", {{"x", uvs.min.x}, {"y", uvs.min.y}}},
        {"uvB", {{"x", uvs.max.x}, {"y", uvs.max.y}}}
      };
    }


    nlohmann::json serialize(const Channel& channel)
    {
      return {
        {"active", channel.state() == Channel::State::Active},
        {"uvs", serialize(channel.uvs())},
        {"gammaFactor", channel.gammaFactor()}
      };
    }


    nlohmann::json serialize(const Channels& channels)
    {
      nlohmann::json jsonChannels = nlohmann::json::array();
      for(const auto& channel : channels){
        auto jsonChannel = JsonCast::serialize(channel.second);
        jsonChannel["channelId"] = channel.first;
        jsonChannels.push_back(jsonChannel);
      }

      return jsonChannels;
    }
  }
}
