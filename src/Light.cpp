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


const Light::GamutCoordinates& Light::gamutCoordinates() const
{
  return m_gamutCoordinates;
}


void Light::setState(bool state)
{
  m_state = state;

  m_bridgeData->_notify(shared_from_this(), NotifyReason::STATE);
}


void Light::setColor(const Color& color)
{
  vec3 normalizedRgb = color.toNormalized();

  m_xy = rgbToXY(normalizedRgb);
  m_brightness = glm::length(normalizedRgb) * 255;

  m_bridgeData->_notify(shared_from_this(), NotifyReason::COLOR);
}


vec2 Light::rgbToXY(const vec3& color)
{
  // Following https://gist.github.com/popcorn245/30afa0f98eea1c2fd34d
  vec3 normalizedRgb = color;

  // Apply gamma
  for(int i = 0; i < normalizedRgb.length(); i++){
    auto& channel = normalizedRgb[i];
    channel = (channel > 0.04045f) ? pow((channel + 0.055f) / (1.0f + 0.055f), 2.4f) : (channel / 12.92f);
  }

  // Apply some magic "Wide RGB D65 conversion formula"
  float& r = normalizedRgb.r;
  float& g = normalizedRgb.g;
  float& b = normalizedRgb.b;

  float X = r * 0.649926f + g * 0.103455f + b * 0.197109f;
  float Y = r * 0.234327f + g * 0.743075f + b * 0.022598f;
  float Z = r * 0.000000f + g * 0.053077f + b * 1.035763f;

  float sum = X + Y + Z;
  
  // White coordinates to be neutral in case of black (skip dividing by zero)
  vec2 xy = vec2(0.315f, 0.3312f);

  if(sum != 0.f){
    xy[0] = X / sum;
    xy[1] = Y / sum;
  }

  // ToDo : Check if x,y fit in light gammut

  return xy;
}
