#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Huenicorn/Credentials.hpp>


namespace Huenicorn
{
  /**
   * @brief Wrapper around Mbed-TLS library to provide TLS v1.2 connection to Hue bridge
   *
   * The actual implementation is selected at compile time depending on the
   * Mbed TLS version: DtlsClientMbedTls3Impl.hpp for Mbed TLS 2.x/3.x,
   * DtlsClientMbedTls4Impl.hpp for Mbed TLS 4.x.
   */
  class DtlsClient
  {
  public:
    // Constructor / destructor
    /**
     * @brief DtlsClient constructor
     *
     * @param credentials PSK credentials
     * @param address Address of the DTLS server
     * @param port Port of the DTLS server
     */
    DtlsClient(const Credentials& credentials, const std::string& address, const std::string& port);


    /**
     * @brief DtlsClient destructor
     *
     */
    ~DtlsClient();


    /**
     * @brief Calls all the inner initializations
     *
     */
    void init();


    /**
     * @brief Sends byte buffer to the server
     *
     * @param requestBuffer Byte buffer to send to the server
     * @return true Sending was successful
     * @return false Sending failed
     */
    bool send(const std::vector<char>& requestBuffer);


    /**
     * @brief Returns whether the DTLS handshake completed and the session is usable
     *
     * @return true Session is connected
     * @return false No session (handshake failed or not performed)
     */
    bool isConnected() const;

  private:
    // Private classes
    class Impl;


    // Attributes
    const Credentials m_credentials;
    const std::string m_address;
    const std::string m_port;

    std::unique_ptr<Impl> m_impl;
  };
}
