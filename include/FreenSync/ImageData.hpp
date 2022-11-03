#pragma once

#include <vector>
#include <cstdint>

struct ImageData
{
  std::vector<uint8_t> pixels;
  int width;
  int height;
  int bitsPerPixel;
};