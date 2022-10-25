#pragma once

#include <optional>
#include <nlohmann/json.hpp>

class BridgeData
{
public:
  BridgeData();
  ~BridgeData();

  const nlohmann::json& bridgeData() const;
  const nlohmann::json& lights() const;

private:
  std::string m_bridgeAddress;
  std::optional<std::string> m_apiKey;

  mutable std::optional<nlohmann::json> m_bridgeData;
  mutable std::optional<nlohmann::json> m_lights;

};