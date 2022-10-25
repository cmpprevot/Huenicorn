#pragma once

#include <optional>
#include <nlohmann/json.hpp>

class Communicator
{
public:
  Communicator();
  ~Communicator();

  static nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body);

};