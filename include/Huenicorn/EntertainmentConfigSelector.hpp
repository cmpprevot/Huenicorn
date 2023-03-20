#pragma once

#include <vector>
#include <optional>
#include <string>

#include <Huenicorn/EntertainmentConfig.hpp>


namespace Huenicorn
{
  class EntertainmentConfigSelector
  {
  public:
    EntertainmentConfigSelector(const std::string& username, const std::string& clientkey, const std::string& address);

    // Getters
    const std::string& selectedEntertainmentConfigId() const;
    const EntertainmentConfig& selectedConfig() const;
    bool validSelecion() const;

    // Methods
    bool selectEntertainementConfig(const std::string& entertainmentConfigId);
    void disableStreaming() const;

  private:
    // Private methods
    void _clearConfigs();

    // Attributes
    const std::string m_username;
    const std::string m_clientkey;
    const std::string m_address;

    EntertainmentConfigs m_entertainmentConfigs;
    EntertainmentConfigsIterator m_selectedConfig{nullptr};
  };
}
