#include <Huenicorn/Config.hpp>

#include <iostream>
#include <fstream>

#include <filesystem>

using namespace std;
using namespace nlohmann;

namespace Huenicorn
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


  unsigned Config::refreshRate() const
  {
    return m_refreshRate;
  }


  unsigned Config::subsampleWidth() const
  {
    return m_subsampleWidth;
  }


  unsigned Config::transitionTime_c() const
  {
    return m_transitionTime_c;
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


  void Config::setSubsampleWidth(unsigned subsampleWidth)
  {
    m_subsampleWidth = subsampleWidth;
    save();
  }


  void Config::setRefreshRate(unsigned refreshRate)
  {
    if(refreshRate < 1){
      refreshRate = 1;
    }

    m_refreshRate = refreshRate;
    save();
  }


  void Config::setTransitionTime_c(unsigned transitionTime_c)
  {
    m_transitionTime_c = std::clamp(transitionTime_c, 0u, 10u);
    save();
  }


  void Config::save() const
  {
    json outConfig = {
      {"subsampleWidth", m_subsampleWidth},
      {"refreshRate", m_refreshRate},
      {"transitionTime_c", m_transitionTime_c},
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

    if(configRoot.contains("refreshRate")){
      m_refreshRate = configRoot.at("refreshRate");
    }
    
    return !configRoot.empty();
  }
}
