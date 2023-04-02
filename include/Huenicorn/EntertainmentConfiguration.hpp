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
    /**
     * @brief EntertainmentConfiguration constructor
     * 
     * @param name Name of the entertainment configuration
     * @param devices List of devices managed by the entertainment configuration
     * @param channels List of channels managed by the entertainment configuration
     */
    EntertainmentConfiguration(const std::string& name, const Devices& devices, const Channels& channels):
    m_name(name),
    m_devices(devices),
    m_channels(channels)
    {}


    // Getters
    /**
     * @brief Returns the entertainment configuration name
     * 
     * @return const std::string&  entertainment configuration name
     */
    const std::string& name() const
    {
      return m_name;
    }


    /**
     * @brief Returns entertainment configuration devices
     * 
     * @return const Devices& Entertainment configuration devices
     */
    const Devices& devices() const
    {
      return m_devices;
    }


    /**
     * @brief Returns entertainment configuration channels
     * 
     * @return const Channels& Entertainment configuration channels
     */
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
