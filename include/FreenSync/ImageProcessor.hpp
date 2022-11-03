#pragma once

#include <opencv2/opencv.hpp>

#include <cstdint>
#include <vector>

#include <FreenSync/Color.hpp>
#include <FreenSync/ImageData.hpp>


using Colors = std::vector<Color>;


class ImageProcessor
{
public:
  ImageProcessor();
  ~ImageProcessor();

  static void rescale(cv::Mat& img, int targetWidth);

  static cv::Mat getSubImage(const cv::Mat& sourceImage, int x, int y, int width, int height);

  //void processImage(cv::Mat& img);

  Colors getDominantColors(cv::Mat& img, unsigned k = 1);

private:

};
