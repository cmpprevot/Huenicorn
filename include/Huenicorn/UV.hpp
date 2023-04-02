#pragma once

#include <glm/vec2.hpp>

namespace Huenicorn
{
  using UV = glm::vec2;

  struct UVs
  {
    UV min;
    UV max;
  };


  enum UVCorner
  {
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3
  };
}
