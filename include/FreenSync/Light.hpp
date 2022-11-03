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

  nlohmann::json serialize() const;

  // Setters
  void setState(bool state);
  void setColor(const Color& color);
  void setUVs(const glm::vec2& uvA, const glm::vec2& uvb);


private:
  // Attributes
  BridgeData* m_bridgeData;
  std::string m_id;
  std::string m_name;
  std::string m_productName;
  Color::GamutCoordinates m_gamutCoordinates;

  glm::vec2 m_xy;
  uint8_t m_brightness;
  bool m_state;

  glm::vec2 m_uvA{0, 0};
  glm::vec2 m_uvB{1, 1};

  Color m_lastColor;
};
