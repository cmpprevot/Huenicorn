#pragma once

#include <memory>

#include <nlohmann/json.hpp>

class BridgeData;
using SharedBridgeData = std::shared_ptr<BridgeData>;

class Light;
using SharedLight = std::shared_ptr<Light>;

class Light : public std::enable_shared_from_this<Light>
{
friend BridgeData;

public:
  enum class NotifyReason{
    STATE,
    COLOR
  };

  Light(BridgeData* bridgeData, const std::string& id, const nlohmann::json& data);

  // Getters
  const std::string& id() const;
  bool state() const;

  // Setters
  void setState(bool state);


private:
  BridgeData* m_bridgeData;
  std::string m_id;
  nlohmann::json m_data;

  int m_i = 0; // ToDo : Remove

  bool m_state;
};
