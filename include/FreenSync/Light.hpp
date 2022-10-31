#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <FreenSync/Color.hpp>


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
  const Color::GamutCoordinates& gamutCoordinates() const;

  // Setters
  void setState(bool state);
  void setColor(const Color& color);


private:
  // Attributes
  BridgeData* m_bridgeData;
  std::string m_id;
  Color::GamutCoordinates m_gamutCoordinates;

  glm::vec2 m_xy;
  uint8_t m_brightness;
  bool m_state;

  Color m_lastColor;
};
