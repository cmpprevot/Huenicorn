#pragma once

#include <vector>
#include <unordered_map>


namespace Huenicorn
{
  struct Light
  {
    std::string id;
    std::string name;
    std::string type;
  };


  struct Channel
  {
    uint8_t id;
    float r{1.0};
    float g{0.0};
    float b{0.0};
  };


  class EntertainmentConfig
  {
  public:
    using Lights = std::unordered_map<std::string, Light>;
    using ChannelIds = std::vector<uint8_t>;

    EntertainmentConfig(const std::string& id, const std::string& name, const Lights& lights, const ChannelIds& channelIds):
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


    const Lights& lights() const
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
    Lights m_lights;
    ChannelIds m_channelIds;
  };
}
