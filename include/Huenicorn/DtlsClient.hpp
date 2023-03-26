#pragma once

#include <string>
#include <vector>

#include <mbedtls/net_sockets.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>
#include <mbedtls/timing.h>

#include <Huenicorn/Credentials.hpp>


namespace Huenicorn
{
  class DtlsClient
  {
  static constexpr std::string Hostname = "Hue";
  static constexpr unsigned HandhsakeAttempts = 4;

  public:
    DtlsClient(const Credentials& credentials, const std::string& address, const std::string& port);
    ~DtlsClient();

    void init();

    bool send(const std::vector<char>& requestBuffer);

  private:
    void _initMembers();
    void _initRNG();
    void _initConnection();
    void _initSSL();
    void _handshake();

    void _deallocate();

    // Attributes
    const Credentials m_credentials;
    const std::string m_address;
    const std::string m_port;

    // MbedTLS
    mbedtls_net_context m_serverFd;
    mbedtls_entropy_context m_entropy;
    mbedtls_ctr_drbg_context m_ctrDrbg;
    mbedtls_ssl_context m_ssl;
    mbedtls_ssl_config m_conf;
    mbedtls_x509_crt m_cacert;
    mbedtls_timing_delay_context m_timer;
  };
}
