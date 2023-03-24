#pragma once

#include <memory>
#include <optional>

#include <Huenicorn/IGrabber.hpp>

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

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

    // Getters
    virtual glm::ivec2 displayResolution() const override;
    virtual RefreshRate displayRefreshRate() const override;

    // Methods
    virtual void grabFrameSubsample(cv::Mat& cvImage) override;

  private:
    // Attributes
    std::optional<ImageData> m_imageData;
    std::unique_ptr<XShmSegmentInfo> m_shmInfo;

    XImage* m_ximage{nullptr};
    Display* m_display{nullptr};
  };
}
