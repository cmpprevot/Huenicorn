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
    Config();
    ~Config();

  // Getters
    const std::filesystem::path& configFilePath() const;
    int restServerPort() const;
    const std::optional<std::string>& bridgeAddress() const;
    const std::optional<std::string>& apiKey() const;
    unsigned refreshRate() const;
    unsigned subsampleWidth() const;
    unsigned transitionTime_c() const;

  // Setters
    void setBridgeAddress(const std::string& bridgeAddress);
    void setApiKey(const std::string& apiKey);
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
    std::optional<std::string> m_apiKey;
    unsigned m_refreshRate{10};
    unsigned m_subsampleWidth{64};
    unsigned m_transitionTime_c{1};
  };
}
