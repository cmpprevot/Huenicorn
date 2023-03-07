#pragma once

#include <vector>
#include <unordered_map>

#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Light.hpp>
#include <Huenicorn/UV.hpp>

namespace Huenicorn
{
  class EntertainmentConfig
  {
  public:
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
