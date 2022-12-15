#pragma once

#include <nlohmann/json.hpp>

namespace FreenSync
{
  namespace RequestUtils
  {
    nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body);
  };
}
