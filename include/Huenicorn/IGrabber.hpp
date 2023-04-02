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
    /**
     * @brief IGrabber constructor
     * 
     * @param config Huenicorn configuration
     */
    IGrabber(Config* config):
    m_config(config)
    {}


    /**
     * @brief IGrabber destructor
     * 
     */
    virtual ~IGrabber(){}


    // Getters
    /**
     * @brief Returns the resolution of the selected display
     * 
     * @return Resolution Resolution of the selected display
     */
    virtual Resolution displayResolution() const = 0;


    /**
     * @brief Returns the refresh rate of the display
     * 
     * @return RefreshRate Refresh rate of the display
     */
    virtual RefreshRate displayRefreshRate() const = 0;


    // Methods
    /**
     * @brief Takes a screen capture and returns a subsample of it as bitmap
     * 
     * @param imageData Subsample of screen capture
     */
    virtual void grabFrameSubsample(cv::Mat& imageData) = 0;


    /**
     * @brief Returns a list of available subsample resolutions
     * 
     * @return Resolutions List of available subsample resolutions
     */
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
    /**
     * @brief Computes a list of integer divisors for a given number
     * 
     * @param number 
     * @return Divisors 
     */
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


    /**
     * @brief Outputs a list of integer divisors for given width and height out of divisor list
     * 
     * @param width Width constrain
     * @param height Height constain
     * @param candidateDivisors Divisors to filter
     * @return Divisors 
     */
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
