#pragma once

#include <FreenSync/ImageData.hpp>

#include <glm/vec2.hpp>

namespace FreenSync
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
