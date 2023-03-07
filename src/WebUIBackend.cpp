#include <Huenicorn/WebUIBackend.hpp>

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/JsonCast.hpp>

using namespace nlohmann;
using namespace std;

namespace Huenicorn
{
  WebUIBackend::WebUIBackend(HuenicornCore* huenicornCore):
  IRestServer("webroot"),
  m_huenicornCore(huenicornCore)
  {
    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/availableLights");
      resource->set_method_handler("GET", [this](SharedSession session){_getAvailableChannels(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/syncedLights");
      resource->set_method_handler("GET", [this](SharedSession session){_getSyncedChannels(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/syncedLight/{lightId: .+}");
      resource->set_method_handler("GET", [this](SharedSession session){_getSyncedChannel(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/allLights");
      resource->set_method_handler("GET", [this](SharedSession session){_getAllChannels(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/displayInfo");
      resource->set_method_handler("GET", [this](SharedSession session){_getDisplayInfo(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/transitionTime_c");
      resource->set_method_handler("GET", [this](SharedSession session){_getTransitionTime_c(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setChannelUV/{lightId: .+}");
      resource->set_method_handler("PUT", [this](SharedSession session){_setChannelUV(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setLightGammaFactor/{lightId: .+}");
      resource->set_method_handler("PUT", [this](SharedSession session){_setChannelGammaFactor(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setSubsampleWidth");
      resource->set_method_handler("PUT", [this](SharedSession session){_setSubsampleWidth(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setRefreshRate");
      resource->set_method_handler("PUT", [this](SharedSession session){_setRefreshRate(session);});
      m_service.publish(resource);
    }
    
    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setTransitionTime_c");
      resource->set_method_handler("PUT", [this](SharedSession session){_setTransitionTime_c(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setChannelActivity");
      resource->set_method_handler("POST", [this](SharedSession session){_setChannelActivity(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/saveProfile");
      resource->set_method_handler("POST", [this](SharedSession session){_saveProfile(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/stop");
      resource->set_method_handler("POST", [this](SharedSession session){_stop(session);});
      m_service.publish(resource);
    }

    m_webfileBlackList.insert("setup.html");
  }


  void WebUIBackend::_getAvailableChannels(const SharedSession& /*session*/) const
  {
    /*
    string response = m_huenicornCore->jsonAvailableLights().dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
    */
  }


  void WebUIBackend::_getSyncedChannel(const SharedSession& /*session*/) const
  {
    /*
    string response = m_huenicornCore->jsonSyncedLights().dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
    */
  }


  void WebUIBackend::_getSyncedChannels(const SharedSession& /*session*/) const
  {
    /*
    const auto request = session->get_request();
    string lightId = request->get_path_parameter("lightId");
    SharedSyncedLight syncedLight = m_huenicornCore->syncedLight(lightId);

    json jsonLight = syncedLight ? syncedLight->serialize() : json::object();
    string response = jsonLight.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
    */
  }


  void WebUIBackend::_getAllChannels(const SharedSession& /*session*/) const
  {
    /*
    string response = m_huenicornCore->jsonAllChannels().dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
    */
  }


  void WebUIBackend::_getDisplayInfo(const SharedSession& session) const
  {
    glm::ivec2 screenResolution = m_huenicornCore->screenResolution();
    auto subsampleResolutionCandidates = m_huenicornCore->subsampleResolutionCandidates();

    json jsonSubsampleCandidates = json::array();
    for(const auto& candidate : this->m_huenicornCore->subsampleResolutionCandidates()){
      jsonSubsampleCandidates.push_back({
        {"x", candidate.x},
        {"y", candidate.y}
      });
    }

    json jsonDisplayInfo{
      {"x", screenResolution.x},
      {"y", screenResolution.y},
      {"subsampleWidth", this->m_huenicornCore->subsampleWidth()},
      {"subsampleResolutionCandidates", jsonSubsampleCandidates}
    };

    string response = jsonDisplayInfo.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_getTransitionTime_c(const SharedSession& session) const
  {
    json jsonTransitionTime{
      {"transitionTime", this->m_huenicornCore->transitionTime_c()},
    };

    string response = jsonTransitionTime.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_setChannelUV(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      const auto& request = session->get_request();
      uint8_t channelId = stoi(request->get_path_parameter("channelId"));

      json jsonUV = json::parse(data);

      float x = jsonUV.at("x");
      float y = jsonUV.at("y");
      UVType uvType = static_cast<UVType>(jsonUV.at("type").get<int>());

      const auto& clampedUVs = m_huenicornCore->setChannelUV(channelId, {x, y}, uvType);

      json jsonResponse = {
        {"uvA", {{"x", clampedUVs.min.x}, {"y", clampedUVs.min.y}}},
        {"uvB", {{"x", clampedUVs.max.x}, {"y", clampedUVs.max.y}}}
      };

      string response = jsonResponse.dump();
      
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setChannelGammaFactor(const SharedSession& session) const
  {
    const auto& request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonGammaFactorData = json::parse(data);
      const auto& request = session->get_request();

      uint8_t channelId = stoi(request->get_path_parameter("channelId"));

      float gammaFactor = jsonGammaFactorData.at("gammaFactor");

      if(!m_huenicornCore->setChannelGammaFactor(channelId, gammaFactor)){
        string response = json{
          {"succeeded", false},
          {"error", "invalid channel id"}
        }.dump();
        session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
        return;
      }

      json jsonResponse = json{
        {"succeeded", true},
        {"gammaFactor", gammaFactor}
      };

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setSubsampleWidth(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());

      int subsampleWidth = json::parse(data).get<int>();

      m_huenicornCore->setSubsampleWidth(subsampleWidth);

      glm::ivec2 screenResolution = m_huenicornCore->screenResolution();
      json jsonScreen{
        {"x", screenResolution.x},
        {"y", screenResolution.y},
        {"subsampleWidth", m_huenicornCore->subsampleWidth()}
      };

      string response = jsonScreen.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setRefreshRate(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());

      unsigned refreshRate = json::parse(data).get<unsigned>();

      m_huenicornCore->setRefreshRate(refreshRate);

      json jsonRefreshRate{
        {"refreshRate", m_huenicornCore->refreshRate()}
      };

      string response = jsonRefreshRate.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setTransitionTime_c(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());

      unsigned refreshRate = json::parse(data).get<unsigned>();

      m_huenicornCore->setTransitionTime_c(refreshRate);

      json jsonTransitionTime{
        {"transitionTime_c", m_huenicornCore->transitionTime_c()}
      };

      string response = jsonTransitionTime.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setChannelActivity(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonChannelData = json::parse(data);

      uint8_t channelId = jsonChannelData.at("channelId");
      bool activity = jsonChannelData.at("activity");

      if(!m_huenicornCore->setChannelActivity(channelId, activity)){
        string response = json{
          {"succeeded", false},
          {"error", "invalid channel id"}
        }.dump();
        session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
        return;
      }

      string response = json{
        {"succeeded", true},
        {"channels", JsonCast::serialize(m_huenicornCore->channels())}
      }.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_saveProfile(const SharedSession& session) const
  {
    m_huenicornCore->saveProfile();

    json jsonResponse = {
      "succeeded", true
    };

    string response = jsonResponse.dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_stop(const SharedSession& session) const
  {
    json jsonResponse = {{
      "succeeded", true
    }};

    string response = jsonResponse.dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });

    m_huenicornCore->stop();
  }
}
