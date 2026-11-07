#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include <Huenicorn/Device.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>


namespace Huenicorn
{
  using MembersIds = std::vector<std::string>;
  using ChannelsMembersIds = std::unordered_map<uint8_t, MembersIds>;
  using EntertainmentConfigurationsChannels = std::unordered_map<std::string, ChannelsMembersIds>;


  /**
   * @brief Provides wrappers around the Hue bridge API
   * 
   */
  namespace ApiTools
  {
    /**
     * @brief Loads entertainment configurations from Hue bridge
     * 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return EntertainmentConfigurations List of entertainment configurations
     */
    EntertainmentConfigurations loadEntertainmentConfigurations(const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Loads devices from all entertainment configurations
     * 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return Devices List of entertainment devices
     */
    Devices loadDevices(const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Loads entertainment configurations channels
     * 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return EntertainmentConfigurationsChannels Map of entertainment configurations channels
     */
    EntertainmentConfigurationsChannels loadEntertainmentConfigurationsChannels(const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Resolves members data from list of IDs
     * 
     * @param membersIds Members ids to match
     * @param devices Devices data
     * @return std::vector<Device> List of matched data for each device
     */
    std::vector<Device> matchDevices(const MembersIds& membersIds, const Devices& devices);

    /**
     * @brief Set the streaming state of the entertainment configuration on the Hue bridge
     * 
     * @param entertainmentConfigurationEntry Entertainment configuration to manage
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @param active True for active, false for inactive
     */
    void setStreamingState(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress, bool active);

    /**
     * @brief Returns the streaming state of the entertainment configuration entry
     * 
     * @param entertainmentConfigurationEntry 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return true Streaming is active
     * @return false Streaming is inactive
     */
    bool streamingActive(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Builds a CLIP v2 light resource body from a Huenicorn manual light state
     *
     * @param state State to convert: {"on": bool, "brightness": 0-100, "mode": "color"|"white",
     *              "r"/"g"/"b": 0-255 for mode "color", or "mirek": 153-500 for mode "white"}
     * @return nlohmann::json CLIP v2 body, e.g. {"on": {"on": true}, "dimming": {"brightness": 80},
     *         "color": {"xy": {"x": ..., "y": ...}}} or {"color_temperature": {"mirek": ...}}
     */
    nlohmann::json buildLightStateBody(const nlohmann::json& state);

    /**
     * @brief Applies a light state (power, brightness, color or color temperature) to a list of lights
     *
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @param lightIds IDs of the lights to control
     * @param state State to apply: {"on": bool, "brightness": 0-100, "mode": "color"|"white",
     *               "r"/"g"/"b": 0-255 for mode "color", or "mirek": 153-500 for mode "white"}
     * @return true All lights accepted the state
     * @return false At least one light rejected the state or the bridge was unreachable
     */
    bool setLightsState(const std::string& username, const std::string& bridgeAddress, const MembersIds& lightIds, const nlohmann::json& state);
  }
}
