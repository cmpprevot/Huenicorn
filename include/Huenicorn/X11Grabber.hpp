#pragma once

#include <optional>

#include <Huenicorn/IGrabber.hpp>

#include <glm/vec2.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace Huenicorn
{
  struct ImageData
  {
    std::vector<uint8_t> pixels;
    int width;
    int height;
    int bitsPerPixel;
  };


  class X11Grabber : public IGrabber
  {
  public:
    X11Grabber(Config* config);
    virtual ~X11Grabber();

    virtual void getScreenSubsample(cv::Mat& cvImage) override;
    virtual glm::ivec2 getScreenResolution() const override;

  private:
    std::optional<ImageData> m_imageData;
    Display* m_display;
  };
}
