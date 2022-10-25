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

  m_bridgeAddress = jsonConfig.at("bridgeAddress");

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


const json& BridgeData::lights() const
{
  try{
    m_lights.emplace(bridgeData().at("lights"));
  }
  catch(const json::exception& e){
    cout << e.what() << endl;
    m_lights.emplace(json::object());
  }

  //cout << bridgeData().size() << endl;
  //cout << lights.size() << endl;

  for(const auto& k : bridgeData()){
    //cout << k << endl;
  }

  return m_lights.value();
}


