#pragma once

#include <vector>
#include <unordered_map>

#include <Huenicorn/UV.hpp>

namespace Huenicorn
{
  struct Light
  {
    std::string id;
    std::string name;
    std::string type;
  };


  struct Channel
  {
    UVs uvs;
    float gammaFactor;
    bool active{true};
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

  class EntertainmentConfig
  {
  public:
    using Lights = std::unordered_map<std::string, Light>;

    EntertainmentConfig(const std::string& name, const Lights& lights, const Channels& channels):
    m_name(name),
    m_lights(lights),
    m_channels(channels)
    {

    }


    const std::string& name() const
    {
      return m_name;
    }


    const Lights& lights() const
    {
      return m_lights;
    }


    const Channels& channels() const
    {
      return m_channels;
    }

  private:
    std::string m_name;
    Lights m_lights;
    Channels m_channels;
  };
}
