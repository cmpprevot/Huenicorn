#include <FreenSync/ScreenUtils.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <err.h>

#include <cstring>

void ScreenUtils::getScreenCapture(ImageData& imageData)
{
  Display* display = XOpenDisplay(nullptr);
  Window root = DefaultRootWindow(display);

  XWindowAttributes attributes = {0};
  XGetWindowAttributes(display, root, &attributes);

  imageData.width = attributes.width;
  imageData.height = attributes.height;

  XImage* img = XGetImage(display, root, 0, 0 , imageData.width, imageData.height, AllPlanes, ZPixmap);
  imageData.bitsPerPixel = img->bits_per_pixel;
  imageData.pixels.resize(imageData.width * imageData.height * 4);

  memcpy(imageData.pixels.data(), img->data, imageData.pixels.size());

  XDestroyImage(img);
  XCloseDisplay(display);
}


glm::vec2 ScreenUtils::getScreenResolution()
{
  Display* display;

  int width = 0;
  int height = 0;
  int screenId = 0;

  if(!(display = XOpenDisplay(0))){
    errx(1, "cannot open display '%s'", XDisplayName(0));
  }

  screenId = DefaultScreen(display);
  width = DisplayWidth(display, screenId);
  height = DisplayHeight(display, screenId);

  return glm::vec2(width, height);
}
