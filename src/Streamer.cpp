#include <Huenicorn/Streamer.hpp>

#include <iostream>

using namespace glm;
using namespace std;


namespace Huenicorn
{
  Streamer::Streamer(const std::string& username, const std::string& clientkey, const std::string& address):
    m_dtlsClient(username, clientkey, address, PORT)
  {
    try{
      m_dtlsClient.init();
    }
    catch(const std::exception& e){
      cout << e.what();
    }
  }


  void Streamer::setEntertainmentConfigId(const std::string& entertainmentConfigId)
  {
    m_header.setEntertainmentConfigurationId(entertainmentConfigId);
  }


  void Streamer::streamChannels(const ChannelStreams& channels)
  {
    vector<char> requestBuffer;
    requestBuffer.insert(requestBuffer.end(), (char*)&m_header, (char*)&m_header + sizeof(HuestreamHeader));

    for(const auto channel : channels){
      HuestreamPayload payload;
      payload.channelId = channel.id;
      payload.setR(static_cast<uint16_t>(0xffff * channel.r));
      payload.setG(static_cast<uint16_t>(0xffff * channel.g));
      payload.setB(static_cast<uint16_t>(0xffff * channel.b));

      requestBuffer.insert(requestBuffer.end(), (char*)&payload, (char*)&payload + sizeof(HuestreamPayload));
    }

    m_dtlsClient.send(requestBuffer);
    m_header.sequenceId++;
  }
}
