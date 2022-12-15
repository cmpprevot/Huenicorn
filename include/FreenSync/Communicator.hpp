#pragma once

#include <optional>
#include <nlohmann/json.hpp>

namespace FreenSync
{
  class Communicator
  {
  public:
    Communicator();
    ~Communicator();

    static nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body);

  };
}
