#pragma once

#include <string>

#include <Huenicorn/Credentials.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>


namespace Huenicorn
{
  class EntertainmentConfigurationSelector
  {
  public:
    // Constructor
    EntertainmentConfigurationSelector(const Credentials& credentials, const std::string& address);

    // Getters
    const std::string& currentEntertainmentConfigurationId() const;
    const EntertainmentConfiguration& currentEntertainmentConfiguration() const;
    const EntertainmentConfigurations& entertainmentConfigurations() const;
    bool validSelection() const;

    // Methods
    bool selectEntertainementConfiguration(const std::string& entertainmentConfigurationId);
    void disableStreaming() const;

  private:
    // Private methods
    void _clearConfigs();

    // Attributes
    const Credentials m_credentials;
    const std::string m_address;

    EntertainmentConfigurations m_entertainmentConfigurations;
    EntertainmentConfigurationsIterator m_currentEntertainmentConfiguration{m_entertainmentConfigurations.end()};
  };
}
