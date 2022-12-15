#include <FreenSync/SyncedLight.hpp>

#include <iostream>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp> // ToDo : remove

#include <FreenSync/BridgeData.hpp>


using namespace glm;
using namespace nlohmann;
using namespace std;

SyncedLight::SyncedLight(SharedBridgeData bridgeData, const LightSummary& lightSummary):
m_bridgeData(bridgeData),
m_lightSummary(lightSummary),
m_state(true)
{
}


const std::string& SyncedLight::id() const
{
  return m_lightSummary.id;
}


bool SyncedLight::state() const
{
  return m_state;
}


const Color::GamutCoordinates& SyncedLight::gamutCoordinates() const
{
  return m_lightSummary.gamutCoordinates;
}


const SyncedLight::UVs& SyncedLight::uvs() const
{
  return m_uvs;
}


float SyncedLight::gammaFactor() const
{
  return m_gammaFactor;
}


nlohmann::json SyncedLight::serialize() const
{
  json serialized = {
    {"id", m_lightSummary.id},
    {"name", m_lightSummary.name},
    {"productName", m_lightSummary.productName},
    {"uvs", {
        {
          "uvA", {{"x", m_uvs.min.x}, {"y", m_uvs.min.y}}
        },
        {
          "uvB", {{"x", m_uvs.max.x}, {"y", m_uvs.max.y}}
        }
      }
    },
    {"gammaFactor", m_gammaFactor}
  };

  return serialized;
}


void SyncedLight::setState(bool state)
{
  m_state = state;

  m_bridgeData->_notify(shared_from_this());
}


void SyncedLight::setColor(const Color& color)
{
  if(color == m_lastColor){
    return;
  }
  
  m_lastColor = color;

  m_xy = m_lastColor.toXY(m_lightSummary.gamutCoordinates);

  float correctedBrightness = pow(m_lastColor.brightness(), _gammaExponent());
  m_brightness = static_cast<Color::ChannelDepth>(correctedBrightness * Color::Max);

  m_bridgeData->_notify(shared_from_this());
}


const SyncedLight::UVs& SyncedLight::setUV(UV&& uv, SyncedLight::UVType uvType)
{
  UVs newUVs = m_uvs;
  uv.x = glm::clamp(uv.x, 0.f, 1.f);
  uv.y = glm::clamp(uv.y, 0.f, 1.f);

  switch (uvType)
  {
    case UVType::TopLeft:
    {
      newUVs.min.x = uv.x;
      newUVs.min.y = uv.y;
      newUVs.max.x = glm::max(uv.x, newUVs.max.x);
      newUVs.max.y = glm::max(uv.y, newUVs.max.y);
      break;
    }

    case UVType::TopRight:
    {
      newUVs.max.x = uv.x;
      newUVs.min.y = uv.y;
      newUVs.min.x = glm::min(uv.x, newUVs.min.x);
      newUVs.max.y = glm::max(uv.y, newUVs.max.y);
      break;
    }

    case UVType::BottomLeft:
    {
      newUVs.min.x = uv.x;
      newUVs.max.y = uv.y;
      newUVs.max.x = glm::max(uv.x, newUVs.max.x);
      newUVs.min.y = glm::min(uv.y, newUVs.min.y);
      break;
    }

    case UVType::BottomRight:
    {
      newUVs.max.x = uv.x;
      newUVs.max.y = uv.y;
      newUVs.min.x = glm::min(uv.x, newUVs.min.x);
      newUVs.min.y = glm::min(uv.y, newUVs.min.y);
      break;
    }

    default:
      break;
  }

  std::swap(m_uvs, newUVs);

  return m_uvs;
}


void SyncedLight::setGammaFactor(float gammaFactor)
{
  m_gammaFactor = gammaFactor;
}
