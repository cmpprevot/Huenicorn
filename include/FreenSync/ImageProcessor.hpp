#pragma once

#include <opencv2/opencv.hpp>

#include <cstdint>
#include <vector>

#include <FreenSync/Color.hpp>

struct ImageData
{
  std::vector<uint8_t> pixels;
  int width;
  int height;
  int bitsPerPixel;
};


using Colors = std::vector<Color>;


class ImageProcessor
{
public:
  ImageProcessor();
  ~ImageProcessor();

  static void rescale(cv::Mat& img, int targetWidth);

  static void getScreenCapture(ImageData& imageData);

  void processImage(cv::Mat& img);

  Colors getDominantColors(cv::Mat& img, unsigned k = 1);

private:

};
