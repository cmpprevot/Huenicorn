#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include <FreenSync/Color.hpp>

using Colors = std::vector<Color>;


namespace ImageProcessing
{
  void rescale(cv::Mat& img, int targetWidth);

  cv::Mat getSubImage(const cv::Mat& sourceImage, int x0, int y0, int x1, int y1);

  Colors getDominantColors(cv::Mat& img, unsigned k = 1);
};
