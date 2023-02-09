#include <Huenicorn/X11Grabber.hpp>

#include <err.h>

#include <cstring>
#include <algorithm>

#include <Huenicorn/Config.hpp>
#include <Huenicorn/ImageProcessing.hpp>

namespace Huenicorn
{
  X11Grabber::X11Grabber(Config* config):
  IGrabber(config)
  {
    if(!(m_display = XOpenDisplay(0))){
      errx(1, "cannot open display '%s'", XDisplayName(0));
    }
  }


  X11Grabber::~X11Grabber()
  {
    XCloseDisplay(m_display);
  }


  void X11Grabber::getScreenSubsample(cv::Mat& cvImage)
  {
    Window root = DefaultRootWindow(m_display);

    if(!m_imageData.has_value()){
      XWindowAttributes attributes = XWindowAttributes();
      XGetWindowAttributes(m_display, root, &attributes);
      m_imageData.emplace();
      m_imageData->width = attributes.width;
      m_imageData->height = attributes.height;
      m_imageData->pixels.resize(m_imageData->width * m_imageData->height * 4);
    }

    XImage* ximg = XGetImage(m_display, root, 0, 0 , m_imageData->width, m_imageData->height, AllPlanes, ZPixmap);
    m_imageData->bitsPerPixel = ximg->bits_per_pixel;

    memcpy(m_imageData->pixels.data(), ximg->data, m_imageData->pixels.size());

    cv::Mat img;

    if(m_imageData->bitsPerPixel > 24){
      img = cv::Mat(m_imageData->height, m_imageData->width, CV_8UC4, m_imageData->pixels.data());
      cv::cvtColor(img, img, cv::COLOR_RGBA2RGB);
    }
    else{
      img = cv::Mat(m_imageData->height, m_imageData->width, CV_8UC3, m_imageData->pixels.data());
    }

    ImageProcessing::rescale(img, m_config->subsampleWidth());

    cvImage = std::move(img);

    XDestroyImage(ximg);
  }


  glm::ivec2 X11Grabber::getScreenResolution() const
  {
    int width = 0;
    int height = 0;
    int screenId = 0;

    screenId = DefaultScreen(m_display);
    width = DisplayWidth(m_display, screenId);
    height = DisplayHeight(m_display, screenId);

    return {width, height};
  }
}
