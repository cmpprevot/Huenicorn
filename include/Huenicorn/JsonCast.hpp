#pragma once

#include <nlohmann/json.hpp>

#include <Huenicorn/UV.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Device.hpp>
#include <Huenicorn/ApiTools.hpp>


namespace Huenicorn
{
  namespace JsonCast
  {
    nlohmann::json serialize(const UVs& uvs);
    nlohmann::json serialize(const Device& device);
    nlohmann::json serialize(const Devices& devices);
    nlohmann::json serialize(const Channel& channel);
    nlohmann::json serialize(const Channels& channels);
    nlohmann::json serialize(const MembersIds& membersIds);
    nlohmann::json serialize(const ChannelsMembers& channelsMembers);
  }
}
