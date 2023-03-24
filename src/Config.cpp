#include <Huenicorn/Config.hpp>

#include <iostream>
#include <fstream>

#include <filesystem>


using namespace std;
using namespace nlohmann;


namespace Huenicorn
{
  Config::Config(const std::filesystem::path& settingsRoot):
  m_configFilePath(settingsRoot / "config.json")
  {
    _loadConfigFile();
  }


  Config::~Config(){}


  const std::filesystem::path& Config::configFilePath() const
  {
    return m_configFilePath;
  }


  bool Config::initialSetupOk() const
  {
    return m_bridgeAddress.has_value() && m_username.has_value() && m_clientkey.has_value();
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


  const optional<std::string>& Config::bridgeAddress() const
  {
    return m_bridgeAddress;
  }


  const optional<std::string>& Config::username() const
  {
    return m_username;
  }


  const optional<std::string>& Config::clientkey() const
  {
    return m_clientkey;
  }


  void Config::setBridgeAddress(const std::string& bridgeAddress)
  {
    m_bridgeAddress.emplace(bridgeAddress);
    save();
  }


  void Config::setUsername(const std::string& username)
  {
    m_username.emplace(username);
    save();
  }


  void Config::setClientkey(const std::string& clientkey)
  {
    m_clientkey.emplace(clientkey);
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


  void Config::save() const
  {
    json outConfig = {
      {"subsampleWidth", m_subsampleWidth},
      {"refreshRate", m_refreshRate},
      {"restServerPort", m_restServerPort},
    };

    if(m_bridgeAddress.has_value()){
      outConfig["bridgeAddress"] = m_bridgeAddress.value();
    }

    if(m_username.has_value()){
      outConfig["username"] = m_username.value();
    }

    if(m_clientkey.has_value()){
      outConfig["clientkey"] = m_clientkey.value();
    }

    if(!filesystem::exists(m_configFilePath)){
      filesystem::create_directories(m_configFilePath.parent_path());
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

    if(!configRoot.contains("username")){
      std::cout << "Missing 'username' in config" << endl;
      ready = false;
    }
    else{
      m_username.emplace(configRoot.at("username"));
    }

    if(!configRoot.contains("clientkey")){
      std::cout << "Missing 'clientkey' in config" << endl;
      ready = false;
    }
    else{
      m_clientkey.emplace(configRoot.at("clientkey"));
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
