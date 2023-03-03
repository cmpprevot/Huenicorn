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

    const std::string& entertainmentConfigId() const;
    const EntertainmentConfig& selectedConfig() const;

    void selectEntertainementConfig(const std::string& entertainmentConfig);

  private:
    // Private methods
    void  _loadEntertainmentData();
    void _setStreamActive(const EntertainmentConfig& entertainmentConfig, bool active);

    void _clearConfigs();

    // Attributes
    const std::string m_username;
    const std::string m_clientkey;
    const std::string m_address;

    std::vector<EntertainmentConfig> m_entertainmentConfigs;
    std::optional<EntertainmentConfig> m_selectedConfig;
  };
}
