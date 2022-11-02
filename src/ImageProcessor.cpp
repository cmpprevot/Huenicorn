#include <FreenSync/ImageProcessor.hpp>

#include <algorithm>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

ImageProcessor::ImageProcessor()
{

}


ImageProcessor::~ImageProcessor()
{

}


void ImageProcessor::rescale(cv::Mat& img, int targetWidth)
{
  int sourceHeight = img.rows;
  int sourceWidth = img.cols;

  if(sourceWidth < targetWidth){
    return;
  }

  float scaleRatio = static_cast<float>(targetWidth) / sourceWidth;

  int targetHeight = sourceHeight * scaleRatio;
  cv::resize(img, img, cv::Size(targetWidth, targetHeight), 0, 0, cv::InterpolationFlags::INTER_LINEAR);
}


void ImageProcessor::getScreenCapture(ImageData& imageData)
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


cv::Mat ImageProcessor::getSubImage(const cv::Mat& sourceImage, int x, int y, int width, int height)
{
  cv::Range cols(std::max(0, x), std::min(x + width, sourceImage.cols));
  cv::Range rows(std::max(0, y), std::min(y + height, sourceImage.rows));

  return sourceImage(rows, cols);
}


Colors ImageProcessor::getDominantColors(cv::Mat& img, unsigned k)
{
  Colors dominantColors;
  dominantColors.reserve(k);
  cv::Mat data = img.reshape(1, img.total());
  data.convertTo(data, CV_32F);

  cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0);

  vector<int> labels;
  cv::Mat centers;
  int flags = cv::KmeansFlags::KMEANS_PP_CENTERS;
  double compactness = cv::kmeans(data, k, labels, criteria, 3, flags, centers);

  centers.convertTo(centers, CV_8U);

  for(int i = 0; i < centers.rows; i++){
    const uint8_t* Mi = centers.ptr<uint8_t>(i);
    dominantColors.emplace_back(Mi[2], Mi[1], Mi[0]);
  }

  return dominantColors;
}
