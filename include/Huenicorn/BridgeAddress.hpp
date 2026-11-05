#pragma once

#include <string>


namespace Huenicorn
{
  constexpr auto HttpProtocol = "https://";

  /**
   * @brief Strips any scheme, path and trailing slashes from a raw bridge address,
   * leaving a bare host[:port] suitable for building request URLs
   *
   * @param rawAddress Raw bridge address as entered by the user
   * @return std::string Sanitized bridge address
   */
  std::string sanitizeBridgeAddress(const std::string& rawAddress);
}
