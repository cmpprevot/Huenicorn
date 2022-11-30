#pragma once

#include <string>

#include <FreenSync/Color.hpp>
#include <nlohmann/json.hpp>

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
