#pragma once

// Mbed TLS 2.x/3.x implementation of DtlsClient::Impl (legacy RNG based).
// Selected at compile time in DtlsClient.cpp when MBEDTLS_VERSION_MAJOR < 4.

#include <stdexcept>
#include <string>
#include <vector>

#ifdef MBEDTLS_PLATFORM_C
#include <mbedtls/platform.h>
#endif

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/timing.h>

#include <Huenicorn/Credentials.hpp>
#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  class DtlsClient::Impl
  {
    // Constants
    static constexpr const char* Hostname = "Hue";
    static constexpr unsigned HandshakeAttempts = 4;

    friend class DtlsClient;

  public:
    Impl() = default;

    ~Impl()
    {
      _deallocate();
    }

    bool connected() const
    {
      return isConnected;
    }

  private:
    static void _debugCallback(void* ctx, int level, const char* file, int line, const char* str)
    {
      const char* p, *basename;
      (void)ctx;

      for(p = basename = file; *p != '\0'; p++){
        if(*p == '/' || *p == '\\'){
          basename = p + 1;
        }
      }

      mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
    }


    void _init(const Credentials& credentials, const std::string& address, const std::string& port)
    {
      m_credentials = credentials;
      m_address = address;
      m_port = port;

      mbedtls_debug_set_threshold(4);

      _initMembers();
      //_initDebug();
      _initRNG();
      _initConnection();
      _initSSL();
      _handshake();
    }


    bool _send(const std::vector<char>& requestBuffer)
    {
      int result = mbedtls_ssl_write(&m_ssl, reinterpret_cast<const unsigned char*>(requestBuffer.data()), requestBuffer.size());
      if(result < 0){
        isConnected = false;
      }

      return (
        result == MBEDTLS_ERR_SSL_WANT_READ ||
        result == MBEDTLS_ERR_SSL_WANT_WRITE ||
        result >= 0
      );
    }


    /**
     * @brief Initializes MberdTLS objects
     *
     */
    void _initMembers()
    {
      mbedtls_net_init(&m_serverFd);
      mbedtls_ssl_init(&m_ssl);
      mbedtls_ssl_config_init(&m_conf);
      mbedtls_x509_crt_init(&m_cacert);
      mbedtls_ctr_drbg_init(&m_ctrDrbg);
    }


    void _initDebug()
    {
      mbedtls_ssl_conf_dbg(&m_conf, _debugCallback, NULL);
    }


    /**
     * @brief Initializes the random number generator
     *
     */
    void _initRNG()
    {
      std::string pers = "dtls_client";
      mbedtls_entropy_init(&m_entropy);
      int result = mbedtls_ctr_drbg_seed(
        &m_ctrDrbg,
        mbedtls_entropy_func,
        &m_entropy,
        reinterpret_cast<const unsigned char*>(pers.data()),
        pers.length()
      );

      if(result != 0){
        throw std::runtime_error("mbedtls_ctr_drbg_seed returned: " + std::to_string(result));
      }
    }


    /**
     * @brief Initializes the connection
     *
     */
    void _initConnection()
    {
      int result = mbedtls_net_connect(
        &m_serverFd,
        m_address.c_str(),
        m_port.c_str(),
        MBEDTLS_NET_PROTO_UDP
      );

      if(result != 0){
        throw std::runtime_error("mbedtls_net_connect failed with code: " + std::to_string(result));
      }
    }


    /**
     * @brief Initializes the SSL layer with PSK
     *
     */
    void _initSSL()
    {
      auto pskRawArray = m_credentials.clientkeyBytes();
      auto pskIdRawArray = m_credentials.usernameBytes();

      int result = mbedtls_ssl_config_defaults(
        &m_conf,
        MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_DATAGRAM,
        MBEDTLS_SSL_PRESET_DEFAULT
      );

      if(result != 0){
        throw std::runtime_error("mbedtls_ssl_config_defaults failed with code: " + std::to_string(result));
      }

      mbedtls_ssl_conf_authmode(&m_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
      mbedtls_ssl_conf_ca_chain(&m_conf, &m_cacert, NULL);
      mbedtls_ssl_conf_rng(&m_conf, mbedtls_ctr_drbg_random, &m_ctrDrbg);

      result = mbedtls_ssl_setup(&m_ssl, &m_conf);

      if(result != 0){
        throw std::runtime_error("mbedtls_ssl_setup failed with code: " + std::to_string(result));
      }

      result = mbedtls_ssl_conf_psk(
        &m_conf,
        reinterpret_cast<const unsigned char*>(pskRawArray.data()),
        pskRawArray.size() * sizeof(unsigned char),
        reinterpret_cast<const unsigned char*>(pskIdRawArray.data()),
        pskIdRawArray.size() * sizeof(unsigned char)
      );

      if(result != 0){
        throw std::runtime_error("mbedtls_ssl_conf_psk failed with code: " + std::to_string(result));
      }

      m_ciphers.push_back(MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256);
      m_ciphers.push_back(0);
      mbedtls_ssl_conf_ciphersuites(&m_conf, m_ciphers.data());
      result = mbedtls_ssl_set_hostname(&m_ssl, Hostname);

      if(result != 0){
        throw std::runtime_error("mbedtls_ssl_set_hostname failed with code: " + std::to_string(result));
      }

      mbedtls_ssl_set_bio(
        &m_ssl,
        &m_serverFd,
        mbedtls_net_send,
        mbedtls_net_recv,
        mbedtls_net_recv_timeout
      );

      mbedtls_ssl_set_timer_cb(
        &m_ssl,
        &m_timer,
        mbedtls_timing_set_delay,
        mbedtls_timing_get_delay
      );
    }


    /**
     * @brief Execute handshake
     *
     */
    void _handshake()
    {
      int result;
      for(unsigned attempt = 0; attempt < HandshakeAttempts; attempt++){
        mbedtls_ssl_conf_handshake_timeout(&m_conf, 400, 1000);
        do{
          result = mbedtls_ssl_handshake(&m_ssl);
        }
        while(result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);

        if(result == 0){
          break;
        }
      }

      if(result != 0){
        throw std::runtime_error("mbedtls_ssl_handshake failed with code: " + std::to_string(result));
      }

      isConnected = true;
      Logger::log("Dtls handshake successful");
    }


    /**
     * @brief Free all the allocated MbedTLS members
     *
     */
    void _deallocate()
    {
      isConnected = false;
      mbedtls_net_free(&m_serverFd);
      mbedtls_x509_crt_free(&m_cacert);
      mbedtls_ssl_free(&m_ssl);
      mbedtls_ssl_config_free(&m_conf);
      mbedtls_ctr_drbg_free(&m_ctrDrbg);
      mbedtls_entropy_free(&m_entropy);
    }


    // Attributes
    Credentials m_credentials;
    std::string m_address;
    std::string m_port;

    std::vector<int> m_ciphers;
    bool isConnected{false};

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
