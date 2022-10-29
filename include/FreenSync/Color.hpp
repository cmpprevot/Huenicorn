#pragma once

#include <cstdint>
#include <sstream>

#include <opencv2/opencv.hpp>

#include <glm/vec3.hpp>

class Color
{
  using ChannelDepth = uint8_t;

public:
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


  glm::vec3 toNormalized() const
  {
    float max = static_cast<float>(std::numeric_limits<ChannelDepth>().max());
    return glm::vec3(
      m_r / max,
      m_g / max,
      m_b / max
    );
  }


private:
  ChannelDepth m_r;
  ChannelDepth m_g;
  ChannelDepth m_b;
};