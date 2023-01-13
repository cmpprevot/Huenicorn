#include <Huenicorn/RequestUtils.hpp>

#include <iostream>
#include <sstream>
#include <fstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>


using namespace nlohmann;
using namespace std;


namespace Huenicorn
{

  namespace RequestUtils
  {
    json sendRequest(const std::string& url, const std::string& method, const std::string& body)
    {
      json jsonBody = json::object();
      curlpp::Cleanup cleaner;
      curlpp::Easy request;

      request.setOpt(new curlpp::options::Url(url));
      request.setOpt(new curlpp::options::CustomRequest{method});

      if(body.size() > 0){
        request.setOpt(new curlpp::options::PostFields(body));
        request.setOpt(new curlpp::options::PostFieldSize(body.length()));
      }

      std::ostringstream response;
      request.setOpt(new curlpp::options::WriteStream(&response));

      try{
        request.perform();
      }
      catch(const curlpp::LibcurlRuntimeError& e){
        cout << e.what() << endl;
      }

      json jsonResponse;

      try{
        jsonResponse = json::parse(response.str());
      }
      catch(const json::exception& e){
        cout << e.what() << endl;
      }

      return jsonResponse;
    }
  }
}
