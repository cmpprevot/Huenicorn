#pragma once

#include <string>

#include <nlohmann/json.hpp>


namespace Huenicorn
{
  namespace RequestUtils
  {
    using Headers = std::multimap<std::string, std::string>;

    nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body = "", const Headers& headers = {});
  }
}
