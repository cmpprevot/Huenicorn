#include <Huenicorn/DtlsClient.hpp>

#include <memory>
#include <stdexcept>

#include <mbedtls/version.h>

#if MBEDTLS_VERSION_MAJOR >= 4
#include <Huenicorn/DtlsClientMbedTls4Impl.hpp>
#else
// Mbed TLS 2.x/3.x
#include <Huenicorn/DtlsClientMbedTls3Impl.hpp>
#endif


namespace Huenicorn
{
  DtlsClient::DtlsClient(const Credentials& credentials, const std::string& address, const std::string& port):
    m_credentials(credentials),
    m_address(address),
    m_port(port)
  {}


  DtlsClient::~DtlsClient() = default;


  void DtlsClient::init()
  {
    m_impl = std::make_unique<Impl>();
    m_impl->_init(m_credentials, m_address, m_port);
  }


  bool DtlsClient::send(const std::vector<char>& requestBuffer)
  {
    if(!m_impl){
      return false;
    }

    return m_impl->_send(requestBuffer);
  }
}
