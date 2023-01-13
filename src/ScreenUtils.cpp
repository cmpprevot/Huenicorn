#include <FreenSync/ScreenUtils.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <err.h>

#include <cstring>
#include <algorithm>


namespace FreenSync
{
  void ScreenUtils::getScreenCapture(ImageData& imageData)
  {
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);

    XWindowAttributes attributes = XWindowAttributes();
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


  glm::ivec2 ScreenUtils::getScreenResolution()
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

    return {width, height};
  }


  std::vector<int> _divisors(int number)
  {
    std::vector<int >divisors;
    for(int i = 1; i < number / 2; i++){
      if(number % i == 0){
        divisors.push_back(i);
      }
    }

    divisors.push_back(number);

    return divisors;
  }


  ScreenUtils::Divisors _selectValidDivisors(int width, int height, const ScreenUtils::Divisors& candidateDivisors)
  {
    ScreenUtils::Divisors validDivisors;

    std::copy_if(candidateDivisors.begin(), candidateDivisors.end(), std::back_inserter(validDivisors), [&](int candidateDivisor){
      return (height % (width / candidateDivisor)) != 0;
    });

    return validDivisors;
  }


  std::vector<glm::ivec2> ScreenUtils::subsampleResolutionCandidates()
  {
    auto screenResolution = getScreenResolution();
    auto wDivisors = _divisors(screenResolution.x);
    auto validDivisors = _selectValidDivisors(screenResolution.x, screenResolution.y, wDivisors);

    std::vector<glm::ivec2> subsampleResolutionCandidates;

    for(const auto& validDivisor : validDivisors){
      int width = screenResolution.x / validDivisor;
      int height = (screenResolution.y * width) / screenResolution.x;
      subsampleResolutionCandidates.emplace_back(width, height);
    }

   return subsampleResolutionCandidates;
  }
}
