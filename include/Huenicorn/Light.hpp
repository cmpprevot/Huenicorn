#pragma once

#include <string>

namespace Huenicorn
{
  struct Light
  {
    std::string id;
    std::string name;
    std::string type;
  };

  using Lights = std::unordered_map<std::string, Light>;
}
