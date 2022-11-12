#include <FreenSync/BridgeData.hpp>

#include <iostream>

#include <FreenSync/Communicator.hpp>


using namespace nlohmann;
using namespace std;

#include <fstream>

BridgeData::BridgeData()
{
  ifstream configFile("config.json");
  json jsonConfig = json::parse(configFile);

  m_bridgeAddress = filesystem::path(jsonConfig.at("bridgeAddress"));

  if(!jsonConfig.contains("apiKey")){
    // Todo : Registration procedure
  }

  m_apiKey.emplace(jsonConfig.at("apiKey"));

}


BridgeData::~BridgeData()
{

}


const json& BridgeData::bridgeData() const
{
  if(!m_bridgeData.has_value()){
    string url = m_bridgeAddress;
    url += "/api/" + m_apiKey.value();

    m_bridgeData.emplace(Communicator::sendRequest(url, "GET", ""));
  }

  return m_bridgeData.value();
}


const LightSummaries& BridgeData::lightSummaries()
{
  if(!m_lightSummaries.has_value()){
    const auto& bridge = bridgeData();
    if(bridge.contains("lights")){
      m_lightSummaries.emplace();
      for(const auto& [key, lightData] : bridge.at("lights").items()){
          string name = lightData.at("name");
          string productName = lightData.at("productname");
          const auto& jsonGamutCoordinates = lightData.at("capabilities").at("control").at("colorgamut");

          Color::GamutCoordinates gamutCoordinates;
          for(int i = 0; const auto& jsonGamutCoordinate : jsonGamutCoordinates){
            gamutCoordinates.at(i).x = jsonGamutCoordinate.at(0);
            gamutCoordinates.at(i).y = jsonGamutCoordinate.at(1);
            i++;
          }

          m_lightSummaries.value().insert({key, {key, name, productName, gamutCoordinates}});
      }
    }
  }

  return m_lightSummaries.value();
}


void BridgeData::_notify(SharedSyncedLight light, SyncedLight::NotifyReason reason)
{
  json request{
    {"on", light->m_state},
    {"bri", light->m_brightness},
    {"xy", {light->m_xy.x, light->m_xy.y}}
  };
  
  filesystem::path url = m_bridgeAddress / "api" / m_apiKey.value() / "lights" / light->id() / "state";

  auto response = Communicator::sendRequest(url, "PUT", request.dump());
}
