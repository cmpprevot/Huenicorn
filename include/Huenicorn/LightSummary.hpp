#pragma once

#include <string>

#include <Huenicorn/Color.hpp>
#include <nlohmann/json.hpp>

namespace Huenicorn
{
  struct LightSummary
  {
    std::string id;
    std::string name;
    std::string productName;
    Color::GamutCoordinates gamutCoordinates;

    nlohmann::json serialize() const
    {
      return {
        {"id", id},
        {"name", name},
        {"productName", productName}
      };
    }
  };
}
