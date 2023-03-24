#include <Huenicorn/EntertainmentConfigSelector.hpp>

#include <iostream>

#include <Huenicorn/ApiTools.hpp>


using namespace std;


namespace Huenicorn
{
  EntertainmentConfigSelector::EntertainmentConfigSelector(const std::string& username, const std::string& clientkey, const std::string& address):
  m_username(username),
  m_clientkey(clientkey),
  m_address(address)
  {
    m_entertainmentConfigs = ApiTools::loadEntertainmentConfigurations(m_username, address);
  }


  const std::string& EntertainmentConfigSelector::selectedEntertainmentConfigId() const
  {
    return m_selectedConfig->first;
  }


  const EntertainmentConfig& EntertainmentConfigSelector::selectedConfig() const
  {
    return m_selectedConfig->second;
  }


  bool EntertainmentConfigSelector::validSelecion() const
  {
    return m_selectedConfig != m_entertainmentConfigs.end();
  }


  bool EntertainmentConfigSelector::selectEntertainementConfig(const std::string& entertainmentConfigId)
  {
    if(m_entertainmentConfigs.size() == 0){
      return false;
    }

    if(entertainmentConfigId == ""){
      cout << "Fallback selection" << endl;
      m_selectedConfig = m_entertainmentConfigs.begin();
    }
    else{
      m_selectedConfig = m_entertainmentConfigs.find(entertainmentConfigId);

      if(m_selectedConfig == m_entertainmentConfigs.end()){
        cout << "Invalid selection : " << entertainmentConfigId << endl;
        return false;
      }
    }

    if(ApiTools::streamingActive(*m_selectedConfig, m_username, m_address)){
      disableStreaming();
    }

    ApiTools::setSelectedConfigStreamActivity(true, *m_selectedConfig, m_username, m_address);

    return true;
  }


  void EntertainmentConfigSelector::disableStreaming() const
  {
    ApiTools::setSelectedConfigStreamActivity(false, *m_selectedConfig, m_username, m_address);
  }


  void EntertainmentConfigSelector::_clearConfigs()
  {
    m_entertainmentConfigs.clear();
    m_selectedConfig = m_entertainmentConfigs.end();
  }
}
