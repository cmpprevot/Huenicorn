#include <Huenicorn/Config.hpp>

#include <iostream>
#include <fstream>

#include <filesystem>

#include <Huenicorn/JsonCast.hpp>


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
    return m_bridgeAddress.has_value() && m_credentials.has_value();
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


  const std::string& Config::username() const
  {
    return m_credentials.value().username();
  }


  const std::string& Config::clientkey() const
  {
    return m_credentials.value().clientkey();
  }


  const std::optional<Credentials>& Config::credentials() const
  {
    return m_credentials;
  }


  void Config::setBridgeAddress(const std::string& bridgeAddress)
  {
    m_bridgeAddress.emplace(bridgeAddress);
    save();
  }


  void Config::setCredentials(const std::string& username, const std::string& clientkey)
  {
    m_credentials.emplace(username, clientkey);
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
    json jsonOutConfig = {
      {"subsampleWidth", m_subsampleWidth},
      {"refreshRate", m_refreshRate},
      {"restServerPort", m_restServerPort},
    };

    if(m_bridgeAddress.has_value()){
      jsonOutConfig["bridgeAddress"] = m_bridgeAddress.value();
    }

    if(m_credentials.has_value()){
      jsonOutConfig["credentials"] = JsonCast::serialize(m_credentials.value());
    }

    if(!filesystem::exists(m_configFilePath)){
      filesystem::create_directories(m_configFilePath.parent_path());
    }

    ofstream configFile(m_configFilePath);
    configFile << jsonOutConfig.dump(2) << endl;
  }


  bool Config::_loadConfigFile()
  {
    json jsonConfigRoot = json::object();

    if(filesystem::exists(m_configFilePath)){
      jsonConfigRoot = json::parse(std::ifstream(m_configFilePath));
    }

    if(jsonConfigRoot.contains("restServerPort")){
      m_restServerPort = jsonConfigRoot.at("restServerPort");
    }
    else{
      cout << "Missing 'restServerPort' in config. Falling back to " << m_restServerPort << endl;
    }

    bool ready = true;
    if(!jsonConfigRoot.contains("bridgeAddress")){
      ready = false;
    }
    else{
      m_bridgeAddress.emplace(jsonConfigRoot.at("bridgeAddress"));
    }

    if(!jsonConfigRoot.contains("credentials")){
      return false;
    }
    else{
      const auto& jsonCredentials = jsonConfigRoot.at("credentials");
      if(!jsonCredentials.contains("username")){
        std::cout << "Missing 'username' in config" << endl;
        return false;
      }

      if(!jsonCredentials.contains("clientkey")){
        std::cout << "Missing 'clientkey' in config" << endl;
        return false;
      }

      m_credentials.emplace(jsonCredentials.at("username"), jsonCredentials.at("clientkey"));
    }

    if(!ready){
      return false;
    }

    if(jsonConfigRoot.contains("subsampleWidth")){
      m_subsampleWidth = jsonConfigRoot.at("subsampleWidth");
    }

    if(jsonConfigRoot.contains("refreshRate")){
      m_refreshRate = jsonConfigRoot.at("refreshRate");
    }
    
    return !jsonConfigRoot.empty();
  }
}
