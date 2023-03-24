#pragma once

#include <optional>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace Huenicorn
{
  class Config
  {
  public:
  // Constructor
    Config(const std::filesystem::path& settingsRoot);
    ~Config();

  // Getters
    const std::filesystem::path& configFilePath() const;
    bool initialSetupOk() const;
    int restServerPort() const;
    const std::optional<std::string>& bridgeAddress() const;
    const std::optional<std::string>& username() const;
    const std::optional<std::string>& clientkey() const;
    unsigned refreshRate() const;
    unsigned subsampleWidth() const;

  // Setters
    void setBridgeAddress(const std::string& bridgeAddress);
    void setUsername(const std::string& username);
    void setClientkey(const std::string& clientKey);
    void setSubsampleWidth(unsigned subsampleWidth);
    void setRefreshRate(unsigned refreshRate);

  // Methods
  void save() const;

  private:
  // Private methods
    bool _loadConfigFile();

  // Private attributes
    std::filesystem::path m_configFilePath;

    int m_restServerPort{8080};
    std::optional<std::string> m_bridgeAddress;
    std::optional<std::string> m_username;
    std::optional<std::string> m_clientkey;
    unsigned m_refreshRate{0};
    unsigned m_subsampleWidth{0};
  };
}
