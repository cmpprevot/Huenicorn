#include <Huenicorn/Channel.hpp>

#include <glm/common.hpp>

namespace Huenicorn
{
  Channel::Channel(bool active, const UVs& uvs, float gammaFactor):
  m_active(active),
  m_uvs(uvs),
  m_gammaFactor(gammaFactor)
  {

  }


  bool Channel::active() const
  {
    return m_active || m_pendingShutdown;
  }


  bool Channel::pendingShutdown() const
  {
    return m_pendingShutdown;
  }


  const UVs& Channel::uvs() const
  {
    return m_uvs;
  }
  
  
  float Channel::gammaFactor() const
  {
    return m_gammaFactor;
  }


  void Channel::setActive(bool active)
  {
    m_active = active;

    if(!active){
      m_pendingShutdown = true;
    }
  }


  UVs& Channel::setUV(UV&& uv, UVType uvType)
  {
    UVs newUVs = m_uvs;
    uv.x = glm::clamp(uv.x, 0.f, 1.f);
    uv.y = glm::clamp(uv.y, 0.f, 1.f);

    switch (uvType)
    {
      case UVType::TopLeft:
      {
        newUVs.min.x = uv.x;
        newUVs.min.y = uv.y;
        newUVs.max.x = glm::max(uv.x, newUVs.max.x);
        newUVs.max.y = glm::max(uv.y, newUVs.max.y);
        break;
      }

      case UVType::TopRight:
      {
        newUVs.max.x = uv.x;
        newUVs.min.y = uv.y;
        newUVs.min.x = glm::min(uv.x, newUVs.min.x);
        newUVs.max.y = glm::max(uv.y, newUVs.max.y);
        break;
      }

      case UVType::BottomLeft:
      {
        newUVs.min.x = uv.x;
        newUVs.max.y = uv.y;
        newUVs.max.x = glm::max(uv.x, newUVs.max.x);
        newUVs.min.y = glm::min(uv.y, newUVs.min.y);
        break;
      }

      case UVType::BottomRight:
      {
        newUVs.max.x = uv.x;
        newUVs.max.y = uv.y;
        newUVs.min.x = glm::min(uv.x, newUVs.min.x);
        newUVs.min.y = glm::min(uv.y, newUVs.min.y);
        break;
      }

      default:
        break;
    }

    std::swap(m_uvs, newUVs);

    return m_uvs;
  }


  void Channel::setGammaFactor(float gammaFactor)
  {
    m_gammaFactor = gammaFactor;
  }
}