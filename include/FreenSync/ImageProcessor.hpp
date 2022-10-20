#pragma once

#include <opencv2/opencv.hpp>

#include <cstdint>
#include <vector>


struct ImageData
{
  std::vector<uint8_t> pixels;
  int width;
  int height;
  int bitsPerPixel;
};

class Color
{
public:
  Color(uint8_t r = 0, uint8_t g = 0,  uint8_t b = 0):
  m_r(r),
  m_g(g),
  m_b(b)
  {}


  std::string toStr() const
  {
    std::stringstream ss;
    ss << (int)m_r << " ";
    ss << (int)m_g << " ";
    ss << (int)m_b;

    return ss.str();
  }


  cv::Scalar toScalar() const
  {
    return cv::Scalar(m_r, m_g, m_b);
  }


private:
  uint8_t m_r;
  uint8_t m_g;
  uint8_t m_b;
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
