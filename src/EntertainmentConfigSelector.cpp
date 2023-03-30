#include <Huenicorn/EntertainmentConfigSelector.hpp>

#include <iostream>

#include <Huenicorn/ApiTools.hpp>


using namespace std;


namespace Huenicorn
{
  EntertainmentConfigSelector::EntertainmentConfigSelector(const Credentials& credentials, const std::string& address):
  m_credentials(credentials),
  m_address(address)
  {
    m_entertainmentConfigs = ApiTools::loadEntertainmentConfigurations(m_credentials.username(), address);
  }


  const std::string& EntertainmentConfigSelector::selectedEntertainmentConfigId() const
  {
    return m_selectedConfig->first;
  }


  const EntertainmentConfig& EntertainmentConfigSelector::selectedConfig() const
  {
    return m_selectedConfig->second;
  }


  const EntertainmentConfigs& EntertainmentConfigSelector::entertainmentConfigs() const
  {
    return m_entertainmentConfigs;
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

    disableStreaming();

    if(entertainmentConfigId.empty()){
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

    // Disable previous configuration
    if(ApiTools::streamingActive(*m_selectedConfig, m_credentials.username(), m_address)){
      disableStreaming();
    }

    ApiTools::setSelectedConfigStreamActivity(true, *m_selectedConfig, m_credentials.username(), m_address);

    return true;
  }


  void EntertainmentConfigSelector::disableStreaming() const
  {
    if(m_selectedConfig == m_entertainmentConfigs.end()){
      return;
    }

    ApiTools::setSelectedConfigStreamActivity(false, *m_selectedConfig, m_credentials.username(), m_address);
  }


  void EntertainmentConfigSelector::_clearConfigs()
  {
    m_entertainmentConfigs.clear();
    m_selectedConfig = m_entertainmentConfigs.end();
  }
}
