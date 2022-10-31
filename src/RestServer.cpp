#include <FreenSync/RestServer.hpp>

#include <iostream>

using namespace std;

RestServer::RestServer(SharedFreenSync freenSync, int port):
m_freenSync(freenSync)
{
  m_settings = make_shared<restbed::Settings>();
  m_settings->set_port(port);
  m_settings->set_default_headers({
    {"Connection", "close"},
    {"Content-Type", "text/plain"},
    //{"", "close"},
  });


  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/");
    resource->set_method_handler("GET", [this](SharedSession session){_getHandler(session);});
    m_service.publish(resource);
  }
}


bool RestServer::start()
{
  if(m_serviceThread.has_value()){
    return false;
  }

  m_serviceThread.emplace([this](){m_service.start(m_settings);});
  cout << "Started web UI" << endl;

  return true;
}


bool RestServer::stop()
{
  if(!m_serviceThread.has_value()){
    return false;
  }

  m_service.stop();
  m_serviceThread.value().join();

  return true;
}


void RestServer::_getHandler(const SharedSession& session)
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
    const auto& request = session->get_request();

    string response = "Hello !";
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}
