#include <Huenicorn/BridgeData.hpp>

#include <iostream>
#include <filesystem>

#include <Huenicorn/RequestUtils.hpp>


using namespace nlohmann;
using namespace std;

#include <fstream>

namespace Huenicorn
{
  BridgeData::BridgeData(const Config& config):
  m_config(config)
  {
    if(!config.bridgeAddress().has_value()){
      cout << "Error : no bridge address was provided in configuration file" << endl;
      return;
    }
    
    if(!config.apiKey().has_value()){
      cout << "Error : no API key was provided in configuration file" << endl;
      return;
    }
  }


  BridgeData::~BridgeData()
  {

  }


  const json& BridgeData::bridgeData() const
  {
    if(!m_bridgeData.has_value()){
      string url = m_config.bridgeAddress().value();
      url += "/api/" + m_config.apiKey().value();

      m_bridgeData.emplace(RequestUtils::sendRequest(url, "GET", ""));
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
      {"transitiontime", m_config.transitionTime_c()}
    };
    
    filesystem::path url = m_config.bridgeAddress().value() + "/api/" + m_config.apiKey().value() + "/lights/" + light->id() + "/state";

    RequestUtils::sendRequest(url, "PUT", request.dump());
  }
}
