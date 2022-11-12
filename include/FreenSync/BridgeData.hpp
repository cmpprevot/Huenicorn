#pragma once

#include <optional>
#include <nlohmann/json.hpp>

#include <FreenSync/SyncedLight.hpp>
#include <FreenSync/LightSummary.hpp>

using LightSummaries = std::unordered_map<std::string, LightSummary>;

class BridgeData
{
friend SyncedLight;

public:
  BridgeData();
  ~BridgeData();

  const nlohmann::json& bridgeData() const;
  const LightSummaries& lightSummaries();



private:
  void _notify(SharedSyncedLight light, SyncedLight::NotifyReason reason);

  std::filesystem::path m_bridgeAddress;
  std::optional<std::string> m_apiKey;

  mutable std::optional<nlohmann::json> m_bridgeData;

  mutable std::optional<LightSummaries> m_lightSummaries;

};