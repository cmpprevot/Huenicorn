#pragma once

#include <string>

#include <FreenSync/Color.hpp>

struct LightSummary
{
  std::string id;
  std::string name;
  std::string productName;
  Color::GamutCoordinates gamutCoordinates;
};
