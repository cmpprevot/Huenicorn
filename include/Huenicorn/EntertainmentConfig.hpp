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
    uint8_t id;
    bool active{true};
    float r{1.0};
    float g{0.0};
    float b{0.0};
    UVs uvs;
  };


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
    using Channels = std::vector<Channel>;

    EntertainmentConfig(const std::string& id, const std::string& name, const Lights& lights, const Channels& channels):
    m_id(id),
    m_name(name),
    m_lights(lights),
    m_channels(channels)
    {

    }


    const std::string& id() const
    {
      return m_id;
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
    std::string m_id;
    std::string m_name;
    Lights m_lights;
    Channels m_channels;
  };
}
