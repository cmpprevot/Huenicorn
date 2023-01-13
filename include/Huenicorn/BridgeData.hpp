#pragma once

#include <optional>
#include <nlohmann/json.hpp>

#include <Huenicorn/Config.hpp>
#include <Huenicorn/SyncedLight.hpp>
#include <Huenicorn/LightSummary.hpp>

namespace Huenicorn
{
  using LightSummaries = std::unordered_map<std::string, LightSummary>;

  class BridgeData
  {
  friend SyncedLight;

  public:
    BridgeData(const Config& config);
    ~BridgeData();

    const nlohmann::json& bridgeData() const;
    const LightSummaries& lightSummaries();



  private:
    void _notify(SharedSyncedLight light);

    const Config& m_config;

    mutable std::optional<nlohmann::json> m_bridgeData;
    mutable std::optional<LightSummaries> m_lightSummaries;
  };
}
