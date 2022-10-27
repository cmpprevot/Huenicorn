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

    std::cout << url << std::endl;

    m_bridgeData.emplace(Communicator::sendRequest(url, "GET", ""));
  }

  return m_bridgeData.value();
}


const Lights& BridgeData::lights()
{
  if(!m_lights.has_value()){
    const auto& bridge = bridgeData();
    if(bridge.contains("lights")){
      m_lights.emplace();
      for(const auto& [key, lightData] : bridge.at("lights").items()){
        m_lights.value().push_back(make_shared<Light>(this, key, lightData));
      }
    }
  }

  return m_lights.value();
}


void BridgeData::_notify(SharedLight light, Light::NotifyReason reason)
{
  //https://<bridge ip address>/api/1028d66426293e821ecfd9ef1a0731df/lights/1/state
  json request = json::object();
  //request["on"] = light->state();
  //request["sat"] = 254;
  //request["bri"] = 254;
  //request["hue"] = 10000;


  request["sat"] = 89;
  request["bri"] = 82;
  request["hue"] = light->m_i;
  light->m_i += 100;
  
  filesystem::path url = m_bridgeAddress / "api" / m_apiKey.value() / "lights" / light->id() / "state";

  //cout << url << endl;
  cout << request.dump() << endl;

  auto data = Communicator::sendRequest(url, "PUT", request.dump());
  cout << data.dump() << endl;
}
