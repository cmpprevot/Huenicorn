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


      void setEntertainmentConfigurationId(const std::string& entertainmentConfiguationId_)
      {
        for(int i = 0; const char& idChar : entertainmentConfiguationId_){
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
    Streamer(const std::string& username, const std::string& clientkey, const std::string& address, const std::string& port);
    ~Streamer();

    bool start();
    void stop();

  private:

    // Private methods
    void  _loadEntertainmentData();

    void _setStreamActive(const EntertainmentConfig& entertainmentConfig, bool active);

    void _selectEntertainementConfig();


    const std::string& _entertainmentId();
    void _streamingLoop();

    // Attributes
    std::optional<std::thread> m_streamThread;

    const std::string m_username;
    const std::string m_clientkey;
    const std::string m_address;

    DtlsClient m_dtlsClient;

    bool m_keepStreaming{false};

    HuestreamHeader m_header;



    std::vector<EntertainmentConfig> m_entertainmentConfigs;
    std::optional<EntertainmentConfig> m_selectedConfig;
  };
}
