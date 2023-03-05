#pragma once

#include <vector>
#include <optional>
#include <string>

#include <Huenicorn/EntertainmentConfig.hpp>


namespace Huenicorn
{
  class EntertainmentConfigSelector
  {
    using EntertainmentConfigs = std::unordered_map<std::string, EntertainmentConfig>;
    using EntertainmentConfigsIterator = EntertainmentConfigs::iterator;


  public:
    EntertainmentConfigSelector(const std::string& username, const std::string& clientkey, const std::string& address);

    const std::string& selectedEntertainmentConfigId() const;
    const EntertainmentConfig& selectedConfig() const;
    bool validSelecion() const;

    bool selectEntertainementConfig(const std::string& entertainmentConfigId);

  private:
    // Private methods
    void  _loadEntertainmentData();
    void _setSelectedConfigStreamActivity(bool active) const;

    void _clearConfigs();

    // Attributes
    const std::string m_username;
    const std::string m_clientkey;
    const std::string m_address;

    EntertainmentConfigs m_entertainmentConfigs;
    EntertainmentConfigsIterator m_selectedConfig{nullptr};
  };
}
