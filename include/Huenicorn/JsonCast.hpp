#pragma once

#include <nlohmann/json.hpp>

#include <Huenicorn/UV.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Light.hpp>


namespace Huenicorn
{
  namespace JsonCast
  {
    nlohmann::json serialize(const UVs& uvs);
    nlohmann::json serialize(const Channel& channel);
    nlohmann::json serialize(const Channels& channels);
  }
}
