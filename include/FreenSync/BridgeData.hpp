#pragma once

#include <optional>
#include <nlohmann/json.hpp>

#include <FreenSync/Config.hpp>
#include <FreenSync/SyncedLight.hpp>
#include <FreenSync/LightSummary.hpp>

namespace FreenSync
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
