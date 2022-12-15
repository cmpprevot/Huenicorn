#pragma once

#include <vector>
#include <cstdint>

namespace FreenSync
{
  struct ImageData
  {
    std::vector<uint8_t> pixels;
    int width;
    int height;
    int bitsPerPixel;
  };
}
