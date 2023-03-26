#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>


namespace Huenicorn
{
  std::vector<unsigned char> hexStringToBytes(const std::string& hexString);
  std::vector<unsigned char> stringToBytes(const std::string& string);


  class Credentials
  {
  public:
    // Constructor
    Credentials(const std::string& username, const std::string& clientkey);

    // Getters
    const std::string& username() const;
    const std::string& clientkey() const;
    std::vector<unsigned char> usernameBytes() const;
    std::vector<unsigned char> clientkeyBytes() const;


  private:
    // Attributes
    std::string m_username;
    std::string m_clientkey;
  };
}
