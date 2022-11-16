#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <FreenSync/LightSummary.hpp>
#include <FreenSync/Color.hpp>


class BridgeData;
using SharedBridgeData = std::shared_ptr<BridgeData>;

class SyncedLight;
using SharedSyncedLight = std::shared_ptr<SyncedLight>;

class SyncedLight : public std::enable_shared_from_this<SyncedLight>
{
friend BridgeData;

public:
  using UVs = std::pair<glm::vec2, glm::vec2>;

  SyncedLight(SharedBridgeData bridgeData, const std::string& id, const LightSummary& lightSummary);

  // Getters
  const std::string& id() const;
  bool state() const;
  const Color::GamutCoordinates& gamutCoordinates() const;
  const UVs& uvs() const;

  nlohmann::json serialize() const;

  // Setters
  void setState(bool state);
  void setColor(const Color& color);
  void setUVs(const glm::vec2& uvA, const glm::vec2& uvb);


private:
  // Attributes
  SharedBridgeData m_bridgeData;
  LightSummary m_lightSummary;

  glm::vec2 m_xy;
  uint8_t m_brightness;
  bool m_state;


  UVs m_uvs{glm::vec2(0), glm::vec2(1)};

  Color m_lastColor;
};
