#pragma once

// Mbed TLS 4.x implementation of DtlsClient::Impl (PSA crypto based).
// Selected at compile time in DtlsClient.cpp when MBEDTLS_VERSION_MAJOR >= 4.
// Ported from gitlab.com/openjowelsofts/huenicorn commit d9891d4.

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <psa/crypto.h>

#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/timing.h>

#include <mbedtls/debug.h>
#include <mbedtls/error.h>

#ifdef MBEDTLS_PLATFORM_C
#include <mbedtls/platform.h>
#endif

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
    template <auto FreeFunc>
    struct MbedTlsDeleter
    {
      template <typename T>
      void operator()(T* ptr) const noexcept
      {
        if(ptr){
          FreeFunc(ptr);
          ptr = nullptr;
        }
      }
    };

    template <typename T, auto FreeFunc>
    using MbedTlsUniquePtr = std::unique_ptr<T, MbedTlsDeleter<FreeFunc>>;

    using UniqueNetContext = MbedTlsUniquePtr<mbedtls_net_context, mbedtls_net_free>;
    using UniqueCert = MbedTlsUniquePtr<mbedtls_x509_crt, mbedtls_x509_crt_free>;
    using UniqueSsl = MbedTlsUniquePtr<mbedtls_ssl_context, mbedtls_ssl_free>;
    using UniqueSslConfig = MbedTlsUniquePtr<mbedtls_ssl_config, mbedtls_ssl_config_free>;

    UniqueNetContext serverFd;
    UniqueSsl ssl;
    UniqueSslConfig conf;
    UniqueCert cacert;
    mbedtls_timing_delay_context timer;
    std::vector<int> ciphers;
    bool isConnected{false};

    Credentials m_credentials;
    std::string m_address;
    std::string m_port;


    static void _debugCallback(
      void* ctx,
      int level,
      const char* file,
      int line,
      const char* str
    )
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


    void _ensurePsaInit()
    {
      static const psa_status_t status = psa_crypto_init();

      if(status != PSA_SUCCESS){
        throw std::runtime_error("psa_crypto_init failed with code: " + std::to_string(static_cast<int>(status)));
      }
    }


    void _init(const Credentials& credentials, const std::string& address, const std::string& port)
    {
      m_credentials = credentials;
      m_address = address;
      m_port = port;

      _ensurePsaInit();

      _initMembers();
      _initConnection();
      _initSSL();
      _handshake();
    }


    /**
     * @brief Initializes MbedTLS objects
     *
     */
    void _initMembers()
    {
      serverFd.reset(new mbedtls_net_context{});
      mbedtls_net_init(serverFd.get());

      ssl.reset(new mbedtls_ssl_context{});
      mbedtls_ssl_init(ssl.get());

      conf.reset(new mbedtls_ssl_config{});
      mbedtls_ssl_config_init(conf.get());

      cacert.reset(new mbedtls_x509_crt{});
      mbedtls_x509_crt_init(cacert.get());
    }


    /**
     * @brief Initializes the connection
     *
     */
    void _initConnection()
    {
      int result = mbedtls_net_connect(
        serverFd.get(),
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

      {
        int result = mbedtls_ssl_config_defaults(
          conf.get(),
          MBEDTLS_SSL_IS_CLIENT,
          MBEDTLS_SSL_TRANSPORT_DATAGRAM,
          MBEDTLS_SSL_PRESET_DEFAULT
        );

        if(result != 0){
          throw std::runtime_error("mbedtls_ssl_config_defaults failed with code: " + std::to_string(result));
        }
      }

      mbedtls_ssl_conf_authmode(conf.get(), MBEDTLS_SSL_VERIFY_OPTIONAL);
      mbedtls_ssl_conf_ca_chain(conf.get(), cacert.get(), NULL);

      {
        int result = mbedtls_ssl_conf_psk(
          conf.get(),
          reinterpret_cast<const unsigned char*>(pskRawArray.data()),
          pskRawArray.size() * sizeof(unsigned char),
          reinterpret_cast<const unsigned char*>(pskIdRawArray.data()),
          pskIdRawArray.size() * sizeof(unsigned char)
        );

        if(result != 0){
          throw std::runtime_error("mbedtls_ssl_conf_psk failed with code: " + std::to_string(result));
        }
      }

      ciphers.push_back(MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256);
      ciphers.push_back(0);

      mbedtls_ssl_conf_ciphersuites(conf.get(), ciphers.data());

      {
        int result = mbedtls_ssl_setup(ssl.get(), conf.get());

        if(result != 0){
          throw std::runtime_error("mbedtls_ssl_setup failed with code: " + std::to_string(result));
        }
      }

      {
        int result = mbedtls_ssl_set_hostname(ssl.get(), Hostname);

        if(result != 0){
          throw std::runtime_error("mbedtls_ssl_set_hostname failed with code: " + std::to_string(result));
        }
      }

      mbedtls_ssl_set_bio(
        ssl.get(),
        serverFd.get(),
        mbedtls_net_send,
        mbedtls_net_recv,
        mbedtls_net_recv_timeout
      );

      mbedtls_ssl_set_timer_cb(
        ssl.get(),
        &timer,
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
      int result{0};
      for(unsigned attempt = 0; attempt < HandshakeAttempts; attempt++){
        mbedtls_ssl_conf_handshake_timeout(conf.get(), 400, 1000);
        do{
          result = mbedtls_ssl_handshake(ssl.get());
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


    bool _send(const std::vector<char>& requestBuffer)
    {
      int result = mbedtls_ssl_write(ssl.get(), reinterpret_cast<const unsigned char*>(requestBuffer.data()), requestBuffer.size());
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
     * @brief Free all the allocated MbedTLS members
     *
     */
    void _deallocate()
    {
      // Politely close the DTLS session (best effort). The bridge keeps a
      // single entertainment session per client key and frees it immediately
      // upon close_notify, instead of keeping it until its idle timeout
      // expires (during which any new handshake fails with a timeout)
      if(isConnected){
        mbedtls_ssl_close_notify(ssl.get());
      }

      isConnected = false;
      serverFd.reset();
      cacert.reset();
      ssl.reset();
      conf.reset();
    }
  };
}
