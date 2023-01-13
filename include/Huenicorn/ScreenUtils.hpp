#pragma once

#include <Huenicorn/ImageData.hpp>

#include <glm/vec2.hpp>

namespace Huenicorn
{
  class ScreenUtils
  {
  public:
    using Divisors = std::vector<int>;

    static void getScreenCapture(ImageData& imageData);
    static glm::ivec2 getScreenResolution();
    static std::vector<glm::ivec2> subsampleResolutionCandidates();
  };
}
