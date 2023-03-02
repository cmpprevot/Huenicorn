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
    [[deprecated("V2 API refers this field as 'username'. Use username() instead")]]
    const std::optional<std::string>& apiKey() const;
    const std::optional<std::string>& username() const;
    const std::optional<std::string>& clientkey() const;
    unsigned refreshRate() const;
    unsigned subsampleWidth() const;
    unsigned transitionTime_c() const;

  // Setters
    void setBridgeAddress(const std::string& bridgeAddress);
    [[deprecated("V2 API refers this field as 'username'. Use setUsername() instead")]]
    void setApiKey(const std::string& apiKey);
    void setUsername(const std::string& username);
    void setClientkey(const std::string& clientKey);
    void setSubsampleWidth(unsigned subsampleWidth);
    void setRefreshRate(unsigned refreshRate);
    void setTransitionTime_c(unsigned transitionTime_c);

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
    unsigned m_refreshRate{10};
    unsigned m_subsampleWidth{0};
    unsigned m_transitionTime_c{1};
  };
}
