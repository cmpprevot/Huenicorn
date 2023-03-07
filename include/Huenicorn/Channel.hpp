#pragma once

#include <unordered_map>

#include <Huenicorn/UV.hpp>

namespace Huenicorn
{
  class Channel
  {
  public:
    Channel(bool active = false, const UVs& uvs = {}, float gammaFactor = 0.f);

    // Getters
    bool active() const;
    const UVs& uvs() const;

    // Setters
    void setActive(bool active);
    UVs& setUV(UV&& uv, UVType uvType);
    void setGammaFactor(float gammaFactor);
    float gammaFactor() const;

  private:
    bool m_active{false};
    UVs m_uvs{{0, 0}, {0, 0}};
    float m_gammaFactor{1.0};
    float m_r{0.0};
    float m_g{0.0};
    float m_b{0.0};
  };

  using Channels = std::unordered_map<uint8_t, Channel>;

  struct ChannelStream
  {
    uint8_t id;
    float r{1.0};
    float g{0.0};
    float b{0.0};
  };
}
