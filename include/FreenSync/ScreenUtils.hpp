#pragma once

#include <FreenSync/ImageData.hpp>

#include <glm/vec2.hpp>

class ScreenUtils
{
public:
  static void getScreenCapture(ImageData& imageData);
  static glm::vec2 getScreenResolution();
};
