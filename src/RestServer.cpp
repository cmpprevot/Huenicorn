#include <FreenSync/RestServer.hpp>

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

#include <FreenSync/FreenSync.hpp>

using namespace nlohmann;
using namespace std;

RestServer::RestServer(SharedFreenSync freenSync, int port):
m_freenSync(freenSync),
m_webroot("webroot")
{
  m_settings = make_shared<restbed::Settings>();
  m_settings->set_port(port);
  m_settings->set_default_headers({
    {"Connection", "close"},
    {"Content-Type", "text/plain"},
  });

  m_contentTypes = {
    {".js", "text/javascript"},
    {".html", "text/html"},
    {".css", "text/css"}
  };

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/");
    resource->set_method_handler("GET", [this](SharedSession session){_getWebFile(session);});
    m_service.publish(resource);
  }

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/{webFileName: [a-zA-Z0-9]+(\\.)[a-zA-Z0-9]+}");
    resource->set_method_handler("GET", [this](SharedSession session){_getWebFile(session);});
    m_service.publish(resource);
  }

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/lights");
    resource->set_method_handler("GET", [this](SharedSession session){_getLights(session);});
    m_service.publish(resource);
  }

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/screen");
    resource->set_method_handler("GET", [this](SharedSession session){_getScreen(session);});
    m_service.publish(resource);
  }

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/setLightUVs/{lightId: .+}");
    resource->set_method_handler("PUT", [this](SharedSession session){_setLightUVs(session);});
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


void RestServer::_getLights(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
    const auto& request = session->get_request();

    json jsonLights;
    for(const auto& [lightId, light] : m_freenSync->lights()){
      jsonLights.push_back(light->serialize());
    }

    string response = jsonLights.dump();
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}


void RestServer::_getScreen(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
    glm::vec2 screenResolution = m_freenSync->screenResolution();
    json jsonScreen{
      {"x", screenResolution.x},
      {"y", screenResolution.y}
    };

    string response = jsonScreen.dump();
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}


void RestServer::_getWebFile(const SharedSession& session) const
{
  const auto request = session->get_request();
  filesystem::path webFileName = request->get_path_parameter("webfileName");

  if(webFileName == ""){
    webFileName = "index.html";
  }

  filesystem::path webFileFullPath = m_webroot / webFileName;

  if(!filesystem::exists(webFileFullPath)){
    webFileName = "404.html";
    webFileFullPath = m_webroot / webFileName;
  }

  string extension = webFileName.extension().string();
  string contentType = "text/plain";

  if(m_contentTypes.find(extension) != m_contentTypes.end()){
    contentType = m_contentTypes.at(extension);
  }

  fstream webFile(webFileFullPath);
  string response = string(std::istreambuf_iterator<char>(webFile), std::istreambuf_iterator<char>());

  std::multimap<string, string> headers{
    {"Content-Length", std::to_string(response.size())},
    {"Content-Type", contentType}
  };

  // ToDo MIME headers
  session->close(restbed::OK, response, headers);
}


void RestServer::_setLightUVs(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){

    string data(reinterpret_cast<const char*>(body.data()), body.size());

    const auto& request = session->get_request();
    string lightId = request->get_path_parameter("lightId");

    json jsonUVs = json::parse(data);

    float uvAx = jsonUVs.at("uvA").at("x");
    float uvAy = jsonUVs.at("uvA").at("y");
    float uvBx = jsonUVs.at("uvB").at("x");
    float uvBy = jsonUVs.at("uvB").at("y");

    {
      std::lock_guard lock(m_freenSync->uvMutex());
      m_freenSync->lights().at(lightId)->setUVs({uvAx, uvAy}, {uvBx, uvBy});
    }

    // Todo : Return corrected values
    string response = data;
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}
