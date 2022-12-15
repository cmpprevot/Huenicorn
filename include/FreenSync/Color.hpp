#pragma once

#include <cstdint>
#include <sstream>

#include <opencv2/opencv.hpp>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace FreenSync
{
  class Color
  {
  public:
    using ChannelDepth = uint8_t;
    static constexpr float Max = static_cast<float>(std::numeric_limits<ChannelDepth>().max());

    using GamutCoordinates = std::array<glm::vec2, 3>;

    Color(ChannelDepth r = 0, ChannelDepth g = 0,  ChannelDepth b = 0):
    m_r(r),
    m_g(g),
    m_b(b)
    {}


    bool operator==(const Color& other) const
    {
      return  m_r == other.m_r &&
              m_g == other.m_g &&
              m_b == other.m_b;
    }


    bool operator!=(const Color& other) const
    {
      return  !(*this == other);
    }


    cv::Scalar toScalar() const
    {
      return cv::Scalar(m_r, m_g, m_b);
    }


    glm::vec3 toNormalized() const
    {
      return glm::vec3(
        m_r / Color::Max,
        m_g / Color::Max,
        m_b / Color::Max
      );
    }


    glm::vec2 toXY(const GamutCoordinates& gamutCoordinates) const
    {
      // Following https://gist.github.com/popcorn245/30afa0f98eea1c2fd34d
      glm::vec3 normalizedRgb = this->toNormalized();

      // Apply gamma
      for(int i = 0; i < normalizedRgb.length(); i++){
        auto& channel = normalizedRgb[i];
        channel = (channel > 0.04045f) ? pow((channel + 0.055f) / (1.0f + 0.055f), 2.4f) : (channel / 12.92f);
      }

      // Apply some magic "Wide RGB D65 conversion formula"
      float& r = normalizedRgb.r;
      float& g = normalizedRgb.g;
      float& b = normalizedRgb.b;

      float X = r * 0.649926f + g * 0.103455f + b * 0.197109f;
      float Y = r * 0.234327f + g * 0.743075f + b * 0.022598f;
      float Z = r * 0.000000f + g * 0.053077f + b * 1.035763f;

      float sum = X + Y + Z;
      
      // White coordinates to be neutral in case of black (skip dividing by zero)
      glm::vec2 xy = glm::vec2(0.315f, 0.3312f);

      if(sum != 0.f){
        xy[0] = X / sum;
        xy[1] = Y / sum;
      }

      // Checking xy boundaries
      (void)gamutCoordinates;
      /*
      if(!_xyInGamut(xy, gamutCoordinates)){
        // ToDo implement at some point
        // This cas has not been observed yet
      }
      */
      return xy;
    }


    float brightness() const
    {
      return (m_r * 0.3f + m_g * 0.59f + m_b * 0.11f) / Color::Max;
    }


  private:
    static inline float _sign(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
    {
      return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
    }


    inline static bool _xyInGamut(const glm::vec2& xy, const GamutCoordinates& gamutCoordinates)
    {
      bool has_neg, has_pos;

      const auto& a = gamutCoordinates.at(0);
      const auto& b = gamutCoordinates.at(1);
      const auto& c = gamutCoordinates.at(2);

      float d1 = _sign(xy, a, b);
      float d2 = _sign(xy, b, c);
      float d3 = _sign(xy, c, a);

      has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
      has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

      return !(has_neg && has_pos);
    }

  private:
    ChannelDepth m_r;
    ChannelDepth m_g;
    ChannelDepth m_b;
  };
}
