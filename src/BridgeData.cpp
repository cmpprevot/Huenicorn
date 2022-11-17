#include <FreenSync/BridgeData.hpp>

#include <iostream>
#include <filesystem>

#include <FreenSync/Communicator.hpp>


using namespace nlohmann;
using namespace std;

#include <fstream>

BridgeData::BridgeData(const json& config)
{
  m_bridgeAddress = filesystem::path(config.at("bridgeAddress"));

  if(!config.contains("apiKey")){
    // Todo : Registration procedure
    cout << "Error : no API key was provided in configuration file" << endl;
    return;
  }

  m_apiKey.emplace(config.at("apiKey"));
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


void BridgeData::_notify(SharedSyncedLight light)
{
  json request{
    {"on", light->m_state},
    {"bri", light->m_brightness},
    {"xy", {light->m_xy.x, light->m_xy.y}},
    {"transitiontime", 1}
  };
  
  filesystem::path url = m_bridgeAddress / "api" / m_apiKey.value() / "lights" / light->id() / "state";

  auto response = Communicator::sendRequest(url, "PUT", request.dump());
}
