#pragma once

#include <vector>


namespace Huenicorn
{
  struct DummyLight
  {
    std::string id;
    std::string name;
    std::string type;
  };


  class EntertainmentConfig
  {
  public:
    using DummyLights = std::unordered_map<std::string, DummyLight>;
    using ChannelIds = std::vector<uint8_t>;

    EntertainmentConfig(const std::string& id, const std::string& name, const DummyLights& lights, const ChannelIds& channelIds):
    m_id(id),
    m_name(name),
    m_lights(lights),
    m_channelIds(channelIds)
    {

    }


    const std::string& id() const
    {
      return m_id;
    }


    const std::string& name() const
    {
      return m_name;
    }


    const DummyLights& lights() const
    {
      return m_lights;
    }


    const ChannelIds& channelIds() const
    {
      return m_channelIds;
    }

  private:
    std::string m_id;
    std::string m_name;
    DummyLights m_lights;
    ChannelIds m_channelIds;
  };
}
