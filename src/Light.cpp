#include <FreenSync/Light.hpp>

#include <iostream>

#include <FreenSync/BridgeData.hpp>


using namespace nlohmann;
using namespace std;

Light::Light(BridgeData* bridgeData, const string& id, const json& jsonLight):
m_bridgeData(bridgeData),
m_id(id),
m_data(jsonLight)
{
}


const std::string& Light::id() const
{
  return m_id;
}


bool Light::state() const
{
  return m_state;
}


void Light::setState(bool state)
{
  m_state = state;

  m_bridgeData->_notify(shared_from_this(), NotifyReason::STATE);
}
