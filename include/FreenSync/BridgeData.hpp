#pragma once

#include <optional>
#include <nlohmann/json.hpp>

#include <FreenSync/Light.hpp>

using Lights = std::unordered_map<std::string, SharedLight>;

class BridgeData
{
friend Light;

public:
  BridgeData();
  ~BridgeData();

  const nlohmann::json& bridgeData() const;
  const Lights& lights();



private:
  void _notify(SharedLight light, Light::NotifyReason reason);

  std::filesystem::path m_bridgeAddress;
  std::optional<std::string> m_apiKey;

  mutable std::optional<nlohmann::json> m_bridgeData;

  mutable std::optional<Lights> m_lights;

};