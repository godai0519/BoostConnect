//
// ssl_socket.hpp
// ~~~~~~~~~~
//
// Specialization from socket_base to SSL connection
//

#ifdef USE_SSL_BOOSTCONNECT
#ifndef BOOSTCONNECT_APPLAYER_SSL_SOCKET_HPP
#define BOOSTCONNECT_APPLAYER_SSL_SOCKET_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boostconnect/application_layer/socket_base.hpp>

namespace bstcon{
namespace application_layer{

class ssl_socket : public socket_common<socket_base::ssl_socket_type>
{
public:
    ssl_socket(io_service& io_service, context_type& ctx);
    virtual ~ssl_socket();

    std::string service_protocol() const;

    //SSL通信のコネクション確立(TCPレイヤーでコネクションを行う)
    error_code& connect(endpoint_type& begin, error_code& ec);
    void async_connect(endpoint_type& begin, ConnectHandler handler);

    void handshake(handshake_type type);
    void async_handshake(handshake_type type, HandshakeHandler handler);

    //[(SSLレイヤーの処理 ->] TCPレイヤーの処理
    void close();
    void shutdown(shutdown_type const what);
};

} // namespace application_layer
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/ssl_socket.ipp"
#endif

#endif
#endif
