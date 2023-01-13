#pragma once

#include <nlohmann/json.hpp>

namespace Huenicorn
{
  namespace RequestUtils
  {
    nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body);
  };
}
