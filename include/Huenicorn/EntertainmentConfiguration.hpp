#pragma once

#include <unordered_map>

#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Device.hpp>
#include <Huenicorn/UV.hpp>


namespace Huenicorn
{
  // Type definitions
  class EntertainmentConfiguration;
  using EntertainmentConfigurations = std::unordered_map<std::string, EntertainmentConfiguration>;
  using EntertainmentConfigurationsIterator = EntertainmentConfigurations::iterator;
  using EntertainmentConfigurationEntry = std::pair<std::string, EntertainmentConfiguration>;


  class EntertainmentConfiguration
  {
  public:
    // Constructor
    EntertainmentConfiguration(const std::string& name, const Devices& devices, const Channels& channels):
    m_name(name),
    m_devices(devices),
    m_channels(channels)
    {}


    // Getters
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
    // Attributes
    std::string m_name;
    Devices m_devices;
    Channels m_channels;
  };
}
