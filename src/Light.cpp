#include <FreenSync/Light.hpp>

#include <iostream>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp> // ToDo : remove

#include <FreenSync/BridgeData.hpp>


using namespace glm;
using namespace nlohmann;
using namespace std;

Light::Light(BridgeData* bridgeData, const string& id, const json& jsonLight):
m_bridgeData(bridgeData),
m_id(id),
m_state(true)
{
  /*/
  // ToDo : Remove
  string data = R"(
  {
    "capabilities" : {
      "control" : {
        "colorgamut" : [
          [0.6915, 0.3083],
          [0.1700, 0.7000],
          [0.1532,0.04750]
        ]
      }
    }
  }
  )";

  json jsonData = json::parse(data);
  const auto& gamutCoordinates = jsonData.at("capabilities").at("control").at("colorgamut");
  /*/

  m_name = jsonLight.at("name");
  m_productName = jsonLight.at("productname");
  const auto& gamutCoordinates = jsonLight.at("capabilities").at("control").at("colorgamut");
  //*/

  for(int i = 0; const auto& gamutCoordinate : gamutCoordinates){
    m_gamutCoordinates.at(i).x = gamutCoordinate.at(0);
    m_gamutCoordinates.at(i).y = gamutCoordinate.at(1);
    i++;
  }
}


const std::string& Light::id() const
{
  return m_id;
}


bool Light::state() const
{
  return m_state;
}


const Color::GamutCoordinates& Light::gamutCoordinates() const
{
  return m_gamutCoordinates;
}


nlohmann::json Light::serialize() const
{
  json serialized = {
    {"id", m_id},
    {"name", m_name},
    {"productName", m_productName}
  };

  return serialized;
}


void Light::setState(bool state)
{
  m_state = state;

  m_bridgeData->_notify(shared_from_this(), NotifyReason::STATE);
}


void Light::setColor(const Color& color)
{
  if(color == m_lastColor){
    return;
  }
  
  m_lastColor = color;

  m_xy = m_lastColor.toXY(m_gamutCoordinates);
  m_brightness = glm::length(m_lastColor.toNormalized()) * 255;

  m_bridgeData->_notify(shared_from_this(), NotifyReason::COLOR);
}
