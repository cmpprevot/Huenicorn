#pragma once

#include <string>

#include <Huenicorn/Credentials.hpp>
#include <Huenicorn/EntertainmentConfig.hpp>


namespace Huenicorn
{
  class EntertainmentConfigSelector
  {
  public:
    // Constructor
    EntertainmentConfigSelector(const Credentials& credentials, const std::string& address);

    // Getters
    const std::string& selectedEntertainmentConfigId() const;
    const EntertainmentConfig& selectedConfig() const;
    const EntertainmentConfigs& entertainmentConfigs() const;
    bool validSelecion() const;

    // Methods
    bool selectEntertainementConfig(const std::string& entertainmentConfigId);
    void disableStreaming() const;

  private:
    // Private methods
    void _clearConfigs();

    // Attributes
    const Credentials m_credentials;
    const std::string m_address;

    EntertainmentConfigs m_entertainmentConfigs;
    EntertainmentConfigsIterator m_selectedConfig{m_entertainmentConfigs.end()};
  };
}
