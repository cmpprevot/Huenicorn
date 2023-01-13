#include <FreenSync/Config.hpp>

#include <iostream>
#include <fstream>

#include <filesystem>

using namespace std;
using namespace nlohmann;

namespace FreenSync
{
  Config::Config():
  m_configFilePath("config.json")
  {
    _loadConfigFile();
  }


  Config::~Config(){}


  const std::filesystem::path& Config::configFilePath() const
  {
    return m_configFilePath;
  }


  int Config::restServerPort() const
  {
    return m_restServerPort;
  }


  float Config::refreshRate() const
  {
    return m_refreshRate;
  }


  int Config::subsampleWidth() const
  {
    return m_subsampleWidth;
  }


  const optional<std::string>& Config::bridgeAddress() const
  {
    return m_bridgeAddress;
  }


  const optional<std::string>& Config::apiKey() const
  {
    return m_apiKey;
  }


  void Config::setBridgeAddress(const std::string& bridgeAddress)
  {
    m_bridgeAddress.emplace(bridgeAddress);
    save();
  }


  void Config::setApiKey(const std::string& apiKey)
  {
    m_apiKey.emplace(apiKey);
    save();
  }


  void Config::setSubsampleWidth(int subsampleWidth)
  {
    m_subsampleWidth = subsampleWidth;
    save();
  }


  void Config::save() const
  {
    json outConfig = {
      {"refreshRate", m_refreshRate},
      {"subsampleWidth", m_subsampleWidth},
      {"restServerPort", m_restServerPort},
    };

    if(m_bridgeAddress.has_value()){
      outConfig["bridgeAddress"] = m_bridgeAddress.value();
    }

    if(m_apiKey.has_value()){
      outConfig["apiKey"] = m_apiKey.value();
    }

    ofstream configFile(m_configFilePath);
    configFile << outConfig.dump(2) << endl;
  }


  bool Config::_loadConfigFile()
  {
    json configRoot = json::object();

    if(filesystem::exists(m_configFilePath)){
      configRoot = json::parse(std::ifstream(m_configFilePath));
    }

    if(configRoot.contains("restServerPort")){
      m_restServerPort = configRoot.at("restServerPort");
    }
    else{
      cout << "Missing 'restServerPort' in config. Falling back to " << m_restServerPort << endl;
    }

    bool ready = true;
    if(!configRoot.contains("bridgeAddress")){
      ready = false;
    }
    else{
      m_bridgeAddress.emplace(configRoot.at("bridgeAddress"));
    }

    if(!configRoot.contains("apiKey")){
      std::cout << "Missing 'apiKey' in config" << endl;
      ready = false;
    }
    else{
      m_apiKey.emplace(configRoot.at("apiKey"));
    }

    if(!ready){
      return false;
    }


    if(configRoot.contains("subsampleWidth")){
      m_subsampleWidth = configRoot.at("subsampleWidth");
    }

    return !configRoot.empty();
  }
}
