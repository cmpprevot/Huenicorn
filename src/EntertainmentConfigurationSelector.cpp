#include <Huenicorn/EntertainmentConfigurationSelector.hpp>

#include <iostream>

#include <Huenicorn/ApiTools.hpp>


using namespace std;


namespace Huenicorn
{
  EntertainmentConfigurationSelector::EntertainmentConfigurationSelector(const Credentials& credentials, const std::string& bridgeAddress):
  m_credentials(credentials),
  m_bridgeAddress(bridgeAddress)
  {
    m_entertainmentConfigurations = ApiTools::loadEntertainmentConfigurations(m_credentials.username(), m_bridgeAddress);
  }


  const std::string& EntertainmentConfigurationSelector::currentEntertainmentConfigurationId() const
  {
    return m_currentEntertainmentConfiguration->first;
  }


  const EntertainmentConfiguration& EntertainmentConfigurationSelector::currentEntertainmentConfiguration() const
  {
    return m_currentEntertainmentConfiguration->second;
  }


  const EntertainmentConfigurations& EntertainmentConfigurationSelector::entertainmentConfigurations() const
  {
    return m_entertainmentConfigurations;
  }


  bool EntertainmentConfigurationSelector::validSelection() const
  {
    return m_currentEntertainmentConfiguration != m_entertainmentConfigurations.end();
  }


  bool EntertainmentConfigurationSelector::selectEntertainementConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(m_entertainmentConfigurations.size() == 0){
      return false;
    }

    disableStreaming();

    if(entertainmentConfigurationId.empty()){
      cout << "Fallback selection" << endl;
      m_currentEntertainmentConfiguration = m_entertainmentConfigurations.begin();
    }
    else{
      m_currentEntertainmentConfiguration = m_entertainmentConfigurations.find(entertainmentConfigurationId);

      if(m_currentEntertainmentConfiguration == m_entertainmentConfigurations.end()){
        cout << "Invalid selection : " << entertainmentConfigurationId << endl;
        return false;
      }
    }

    // Disable previous configuration
    if(ApiTools::streamingActive(*m_currentEntertainmentConfiguration, m_credentials.username(), m_bridgeAddress)){
      disableStreaming();
    }

    ApiTools::setStreamingState(*m_currentEntertainmentConfiguration, m_credentials.username(), m_bridgeAddress, true);

    return true;
  }


  void EntertainmentConfigurationSelector::disableStreaming() const
  {
    if(m_currentEntertainmentConfiguration == m_entertainmentConfigurations.end()){
      return;
    }

    ApiTools::setStreamingState(*m_currentEntertainmentConfiguration, m_credentials.username(), m_bridgeAddress, false);
  }
}
