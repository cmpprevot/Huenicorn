#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include <FreenSync/Color.hpp>


namespace FreenSync
{
  using Colors = std::vector<Color>;

  namespace ImageProcessing
  {
    void rescale(cv::Mat& img, int targetWidth);

    cv::Mat getSubImage(const cv::Mat& sourceImage, const glm::ivec2& a, const glm::ivec2& b);

    Colors getDominantColors(cv::Mat& img, unsigned k = 1);
  };
}
