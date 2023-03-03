#pragma once

#include <string>
#include <optional>
#include <thread>
#include <vector>
#include <unordered_map>

#include <Huenicorn/DtlsClient.hpp>
#include <Huenicorn/EntertainmentConfig.hpp>


namespace Huenicorn
{
  static constexpr std::string PORT = "2100";

  class Streamer
  {
    struct HuestreamHeader
    {
      char protocolName[9] = {'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm'};
      char version[2] = {0x02, 0x00};
      char sequenceId = 0;
      char reserved1[2] = {0, 0};
      char colorSpace = 0x00;
      char reserved2 = 0;
      char entertainmentConfiguationId[36];


      void setColorSpace(char _colorSpace)
      {
        this->colorSpace = _colorSpace;
      }


      void setEntertainmentConfigurationId(const std::string& _entertainmentConfiguationId)
      {
        for(int i = 0; const char& idChar : _entertainmentConfiguationId){
          entertainmentConfiguationId[i++] = idChar;
        }
      }
    };


    struct HuestreamPayload
    {
      char channelId;
      char colorData0[2];
      char colorData1[2];
      char colorData2[2];

      void setChannelId(char _channelId)
      {
        channelId = _channelId;
      }


      void setR(uint16_t red)
      {
        uint8_t a = static_cast<uint8_t>((red >> 8) & 0xff);
        uint8_t b = static_cast<uint8_t>(red & 0xff);
        colorData0[0] = a;
        colorData0[1] = b;
      }

      void setG(uint16_t green)
      {
        uint8_t a = static_cast<uint8_t>((green >> 8) & 0xff);
        uint8_t b = static_cast<uint8_t>(green & 0xff);
        colorData1[0] = a;
        colorData2[1] = b;
      }

      void setB(uint16_t blue)
      {
        uint8_t a = static_cast<uint8_t>((blue >> 8) & 0xff);
        uint8_t b = static_cast<uint8_t>(blue & 0xff);
        colorData2[0] = a;
        colorData2[1] = b;
      }
    };


  public:
    Streamer(const std::string& username, const std::string& clientkey, const std::string& address);

    void setEntertainmentConfigId(const std::string& entertainmentConfigId);

    // Methods
    void streamChannels(const std::vector<Channel>& channels);

  private:
    // Attributes
    DtlsClient m_dtlsClient;
    HuestreamHeader m_header;

  };
}
