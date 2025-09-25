#pragma once

#include <unordered_map>
#include <vector>

#include <glm/exponential.hpp>

#include <Huenicorn/Device.hpp>
#include <Huenicorn/UV.hpp>


namespace Huenicorn
{
  // Type definitions
  class Channel;
  using Channels = std::unordered_map<uint8_t, Channel>;


  /**
   * @brief Channel structure matching Hue streaming protocol
   * 
   */
  struct ChannelStream
  {
    uint8_t id;
    float r{0.0};
    float g{0.0};
    float b{0.0};
  };
  using ChannelStreams = std::vector<ChannelStream>;


  /**
   * @brief Wrapper around Hue Channel extended with UV and Gamma control
   * 
   */
  class Channel
  {
  public:
    // Type definitions
    enum class State
    {
      Inactive,
      Active,
      PendingShutdown
    };

    /**
     * @brief Channel constructor
     * 
     * @param active Whether the channel is active or not
     * @param devices List of devices driven by the channel
     * @param gammaFactor Gamma factor of the light
     * @param uvs UVs of the screen portion
     */
    Channel(bool active, const std::vector<Device>& devices, float gammaFactor, const UVs& uvs = {{0, 0}, {1, 1}});


    // Getters
    /**
     * @brief Returns the current state of the channel
     * 
     * @return State Channel state
     */
    State state() const;

    /**
     * @brief Returns the UVs of the channel
     * 
     * @return const UVs& Channel UVs
     */
    const UVs& uvs() const;

    /**
     * @brief Returns the gammaFactor of the channel
     * 
     * @return float Channel gamma factor
     */
    float gammaFactor() const;

    /**
     * @brief Returns a list of member devices
     * 
     * @return const std::vector<Device>& 
     */
    const std::vector<Device>& devices() const;

    /**
     * @brief Returns the exponent factor for gamma
     * 
     * @return float Channel gamma factor
     */
    inline float gammaExponent() const
    {
      float factor = 2.f;
      float exponent = glm::pow(2, -m_gammaFactor * factor);
      return exponent;
    }


    // Setters
    /**
     * @brief Set the channel activity
     * 
     * @param active Whether the channel must be active or not
     */
    void setActive(bool active);

    /**
     * @brief Sets the UVs of the channel
     * 
     * @param uv UV coordinate to set
     * @param uvCorner Specified corner
     * @return UVs& Clamped UVs
     */
    UVs& setUV(UV&& uv, UVCorner uvCorner);

    /**
     * @brief Set the gamma factor 
     * 
     * @param gammaFactor Gamma factor to affect
     */
    void setGammaFactor(float gammaFactor);


    // Methods
    /**
     * @brief Call to move from pending state to inactive
     * 
     */
    void acknowledgeShutdown();


  private:
    // Attributes
    State m_state{State::Inactive};
    std::vector<Device> m_devices;
    float m_gammaFactor{0.0};
    UVs m_uvs{};
  };


  // Serialization
  void to_json(nlohmann::json& jsonChannel, const Channel& channel);
  void to_json(nlohmann::json& jsonChannels, const Channels& channels);
}
