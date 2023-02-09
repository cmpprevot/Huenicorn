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
    using Divisors = std::vector<int>;
    using Resolution = glm::ivec2;
    using Resolutions = std::vector<Resolution>;

    IGrabber(Config* config):
    m_config(config)
    {}

    virtual ~IGrabber(){}

    virtual void getScreenSubsample(cv::Mat& imageData) = 0;
    virtual Resolution getScreenResolution() const = 0;


    Resolutions subsampleResolutionCandidates()
    {
      auto screenResolution = getScreenResolution();
      auto wDivisors = _divisors(screenResolution.x);
      auto validDivisors = _selectValidDivisors(screenResolution.x, screenResolution.y, wDivisors);

      Resolutions subsampleResolutionCandidates;

      for(const auto& validDivisor : validDivisors){
        int width = screenResolution.x / validDivisor;
        int height = (screenResolution.y * width) / screenResolution.x;
        subsampleResolutionCandidates.emplace_back(width, height);
      }

      return subsampleResolutionCandidates;
    }


  protected:
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
