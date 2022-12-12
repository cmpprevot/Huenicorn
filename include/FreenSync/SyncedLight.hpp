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
  using UV = glm::vec2;
  struct UVs{
    UV min;
    UV max;
  };


  enum UVType
  {
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3
  };

  SyncedLight(SharedBridgeData bridgeData, const std::string& id, const LightSummary& lightSummary);

  // Getters
  const std::string& id() const;
  bool state() const;
  const Color::GamutCoordinates& gamutCoordinates() const;
  const UVs& uvs() const;
  float gammaFactor() const;

  nlohmann::json serialize() const;

  // Setters
  void setState(bool state);
  void setColor(const Color& color);
  const UVs& setUV(UV&& uv, UVType uvType);
  void setGammaFactor(float gammaFactor);


private:

  inline float _gammaExponent()
  {
    float factor = 2.f;
    float exponent = pow(2, -m_gammaFactor * factor);
    return exponent;
  }

  // Attributes
  SharedBridgeData m_bridgeData;
  LightSummary m_lightSummary;

  glm::vec2 m_xy;
  uint8_t m_brightness;
  bool m_state;

  float m_gammaFactor{0.f};

  UVs m_uvs{glm::vec2(0), glm::vec2(1)};

  Color m_lastColor;
};
