#pragma once

#include <string>
#include <unordered_map>

namespace Huenicorn
{
  struct Device
  {
    std::string id;
    std::string name;
    std::string type;
  };

  using Devices = std::unordered_map<std::string, Device>;
}
