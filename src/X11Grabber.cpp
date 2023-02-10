#include <Huenicorn/X11Grabber.hpp>

#include <err.h>

#include <cstring>
#include <algorithm>

#include <X11/Xutil.h>
#include <sys/shm.h>

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

    int scr = XDefaultScreen(m_display);

    m_shmInfo = std::make_unique<XShmSegmentInfo>();

    Screen* screen = ScreenOfDisplay(m_display, 0);

    m_ximg = XShmCreateImage(m_display,
      DefaultVisual(m_display, scr),
      DefaultDepth(m_display, scr),
      ZPixmap,
      NULL,
      m_shmInfo.get(),
      screen->width,
      screen->height
    );

    m_shmInfo->shmid = shmget(IPC_PRIVATE, m_ximg->bytes_per_line * m_ximg->height, IPC_CREAT | 0777);

    m_shmInfo->readOnly = False;
    m_shmInfo->shmaddr = m_ximg->data = (char*)shmat(m_shmInfo->shmid, 0, 0);

    XShmAttach(m_display, m_shmInfo.get());
  }


  X11Grabber::~X11Grabber()
  {
    if(m_shmInfo) {
      shmdt(m_shmInfo->shmaddr);
      shmctl(m_shmInfo->shmid, IPC_RMID, 0);
      XShmDetach(m_display, m_shmInfo.get());
      m_shmInfo.reset();
    }

    if(m_ximg){
      XDestroyImage(m_ximg);
      m_ximg = nullptr;
    }

    if(m_display){
      XCloseDisplay(m_display);
      m_display = nullptr;
    }
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

    XShmGetImage(m_display, RootWindow(m_display, DefaultScreen(m_display)), m_ximg, 0, 0, AllPlanes);

    m_imageData->bitsPerPixel = m_ximg->bits_per_pixel;

    memcpy(m_imageData->pixels.data(), m_ximg->data, m_imageData->pixels.size());

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
