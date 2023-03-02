#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace Huenicorn
{
  namespace RequestUtils
  {
    using Headers = std::multimap<std::string, std::string>;

    nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body = "", const Headers& headers = {});
  }
}
