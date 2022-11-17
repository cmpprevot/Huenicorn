#include <FreenSync/SyncedLight.hpp>

#include <iostream>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp> // ToDo : remove

#include <FreenSync/BridgeData.hpp>


using namespace glm;
using namespace nlohmann;
using namespace std;

SyncedLight::SyncedLight(SharedBridgeData bridgeData, const string& id, const LightSummary& lightSummary):
m_bridgeData(bridgeData),
m_state(true),
m_lightSummary(lightSummary)
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


nlohmann::json SyncedLight::serialize() const
{
  json serialized = {
    {"id", m_lightSummary.id},
    {"name", m_lightSummary.name},
    {"productName", m_lightSummary.productName},
    {"uvs", {
        {
          "uvA", {{"x", m_uvs.first.x}, {"y", m_uvs.first.y}}
        },
        {
          "uvB", {{"x", m_uvs.second.x}, {"y", m_uvs.second.y}}
        }
      }
    },
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
  m_brightness = m_lastColor.brightness();

  m_bridgeData->_notify(shared_from_this());
}


void SyncedLight::setUVs(const glm::vec2& uvA, const glm::vec2& uvB)
{
  m_uvs.first = uvA;
  m_uvs.second = uvB;
}
