#pragma once

#include <unordered_map>

#include <Huenicorn/UV.hpp>

namespace Huenicorn
{
  struct Channel
  {
    bool active{true};
    UVs uvs{{0, 0}, {0, 0}};
    float gammaFactor{1.0};
    float r{0.0};
    float g{0.0};
    float b{0.0};
  };

  using Channels = std::unordered_map<uint8_t, Channel>;

  struct ChannelStream
  {
    uint8_t id;
    float r{1.0};
    float g{0.0};
    float b{0.0};
  };
}
