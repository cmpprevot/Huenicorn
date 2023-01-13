#pragma once

#include <optional>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace FreenSync
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
    float refreshRate() const;
    int subsampleWidth() const;

  // Setters
    void setBridgeAddress(const std::string& bridgeAddress);
    void setApiKey(const std::string& apiKey);
    void setSubsampleWidth(int subsampleWidth);

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
    float m_refreshRate{10};
    int m_subsampleWidth{64};
  };
}
