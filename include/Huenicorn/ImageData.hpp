#pragma once

#include <vector>
#include <cstdint>

namespace Huenicorn
{
  struct ImageData
  {
    std::vector<uint8_t> pixels;
    int width;
    int height;
    int bitsPerPixel;
  };
}
