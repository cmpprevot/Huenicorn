#pragma once

#include <optional>
#include <filesystem>

#include <nlohmann/json.hpp>

#include <Huenicorn/Credentials.hpp>


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
    const std::string& username() const;
    const std::string& clientkey() const;
    const std::optional<Credentials>& credentials() const;
    unsigned refreshRate() const;
    unsigned subsampleWidth() const;

  // Setters
    void setBridgeAddress(const std::string& bridgeAddress);
    void setCredentials(const std::string& username, const std::string& clientkey);
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
    std::optional<Credentials> m_credentials;
    unsigned m_refreshRate{0};
    unsigned m_subsampleWidth{0};
  };
}
