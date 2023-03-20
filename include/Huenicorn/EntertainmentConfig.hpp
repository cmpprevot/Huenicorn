#pragma once

#include <vector>
#include <unordered_map>

#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Device.hpp>
#include <Huenicorn/UV.hpp>

namespace Huenicorn
{
  class EntertainmentConfig;
  using EntertainmentConfigs = std::unordered_map<std::string, EntertainmentConfig>;
  using EntertainmentConfigsIterator = EntertainmentConfigs::iterator;
  using EntertainmentConfigEntry = std::pair<std::string, EntertainmentConfig>;


  class EntertainmentConfig
  {
  public:

    EntertainmentConfig(const std::string& name, const Devices& devices, const Channels& channels):
    m_name(name),
    m_devices(devices),
    m_channels(channels)
    {

    }


    const std::string& name() const
    {
      return m_name;
    }


    const Devices& devices() const
    {
      return m_devices;
    }


    const Channels& channels() const
    {
      return m_channels;
    }

  private:
    std::string m_name;
    Devices m_devices;
    Channels m_channels;
  };
}
