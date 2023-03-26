#include <Huenicorn/SetupBackend.hpp>
#include <Huenicorn/HuenicornCore.hpp>

using namespace std;
using namespace nlohmann;


namespace Huenicorn
{
  SetupBackend::SetupBackend(HuenicornCore* core):
  IRestServer("webroot"),
  m_core(core)
  {
    m_indexFile = "setup.html";

    m_contentTypes = {
      {".js", "text/javascript"},
      {".html", "text/html"},
      {".css", "text/css"}
    };

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/finishSetup");
      resource->set_method_handler("POST", [this](SharedSession session){_finish(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/abort");
      resource->set_method_handler("POST", [this](SharedSession session){_abort(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/autoDetectBridge");
      resource->set_method_handler("GET", [this](SharedSession session){_autoDetectBridge(session);});
      m_service.publish(resource);
    }
    
    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/configFilePath");
      resource->set_method_handler("GET", [this](SharedSession session){_configFilePath(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/validateBridgeAddress");
      resource->set_method_handler("PUT", [this](SharedSession session){_validateBridgeAddress(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/validateCredentials");
      resource->set_method_handler("PUT", [this](SharedSession session){_validateCredentials(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/registerNewUser");
      resource->set_method_handler("PUT", [this](SharedSession session){_registerNewUser(session);});
      m_service.publish(resource);
    }

    m_webfileBlackList.insert("index.html");
  }


  SetupBackend::~SetupBackend()
  {}


  bool SetupBackend::aborted() const
  {
    return m_aborted;
  }


  void SetupBackend::_onStart()
  {
    thread spawnBrowserThread([this](){_spawnBrowser();});
    spawnBrowserThread.detach();
  }


  void SetupBackend::_spawnBrowser()
  {
    while (m_service.is_down()){
      this_thread::sleep_for(100ms);
    }
    
    stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_settings->get_port();
    string serviceURL = serviceUrlStream.str();
    std::cout << "Setup WebUI is ready and available at " << serviceURL << std::endl;

    system(string("xdg-open " + serviceURL).c_str());
  }


  void SetupBackend::_finish(const SharedSession& session)
  {
    string response = "{}";
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });

    stop();
  }


  void SetupBackend::_abort(const SharedSession& session)
  {
    string response = "{}";
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });

    m_aborted = true;

    stop();
  }


  void SetupBackend::_autoDetectBridge(const SharedSession& session)
  {
    json jsonResponse = m_core->autoDetectedBridge();
    string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }

  
  void SetupBackend::_configFilePath(const SharedSession& session)
  {
    json jsonResponse = {{"configFilePath", m_core->configFilePath()}};
    string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void SetupBackend::_validateBridgeAddress(const SharedSession& session)
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonBridgeAddressData = json::parse(data);

      string bridgeAddress = jsonBridgeAddressData.at("bridgeAddress");

      json jsonResponse = {{"succeeded", m_core->validateBridgeAddress(bridgeAddress)}};

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void SetupBackend::_validateCredentials(const SharedSession& session)
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonCredentials = json::parse(data);

      Credentials credentials(jsonCredentials.at("username"), jsonCredentials.at("clientkey"));

      json jsonResponse = {{"succeeded", m_core->validateCredentials(credentials)}};

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void SetupBackend::_registerNewUser(const SharedSession& session)
  {
    json jsonResponse = m_core->registerNewUser();
    string response = jsonResponse.dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }
}
