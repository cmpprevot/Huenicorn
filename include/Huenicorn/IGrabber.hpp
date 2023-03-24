#pragma once

#include <algorithm>

#include <opencv2/opencv.hpp>

#include <glm/vec2.hpp>

namespace Huenicorn
{
  class Config;

  class IGrabber
  {
  public:
    // Type definitions
    using Divisors = std::vector<int>;
    using Resolution = glm::ivec2;
    using Resolutions = std::vector<Resolution>;
    using RefreshRate = unsigned;

    struct DisplayInfo
    {
      Resolution resolution;
      RefreshRate refreshRate;
    };

    // Constructor / Destructor
    IGrabber(Config* config):
    m_config(config)
    {}

    virtual ~IGrabber(){}

    // Getters
    virtual Resolution displayResolution() const = 0;
    virtual RefreshRate displayRefreshRate() const = 0;

    // Methods
    virtual void grabFrameSubsample(cv::Mat& imageData) = 0;

    Resolutions subsampleResolutionCandidates()
    {
      auto resolution = displayResolution();
      auto wDivisors = _divisors(resolution.x);
      auto validDivisors = _selectValidDivisors(resolution.x, resolution.y, wDivisors);

      Resolutions subsampleResolutionCandidates;

      for(const auto& validDivisor : validDivisors){
        int width = resolution.x / validDivisor;
        int height = (resolution.y * width) / resolution.x;
        subsampleResolutionCandidates.emplace_back(width, height);
      }

      return subsampleResolutionCandidates;
    }


  protected:
    // Protected static methods
    inline static Divisors _divisors(int number)
    {
      std::vector<int>divisors;
      for(int i = 1; i < number / 2; i++){
        if(number % i == 0){
          divisors.push_back(i);
        }
      }

      divisors.push_back(number);

      return divisors;
    }


    inline static Divisors _selectValidDivisors(int width, int height, const Divisors& candidateDivisors)
    {
      Divisors validDivisors;

      std::copy_if(candidateDivisors.begin(), candidateDivisors.end(), std::back_inserter(validDivisors), [&](int candidateDivisor){
        return (height % (width / candidateDivisor)) != 0;
      });

      return validDivisors;
    }


    //Attributes
    Config* m_config;
  };
}
