//
// ssl_socket.ipp
// ~~~~~~~~~~
//
// Specialization from socket_base to SSL connection
//

#ifdef USE_SSL_BOOSTCONNECT
#ifndef BOOSTCONNECT_APPLAYER_SSL_SOCKET_IPP
#define BOOSTCONNECT_APPLAYER_SSL_SOCKET_IPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "../ssl_socket.hpp"

namespace bstcon{
namespace application_layer{
    
ssl_socket::ssl_socket(io_service& io_service, context_type& ctx) : my_base(io_service, ctx)
{
}
ssl_socket::~ssl_socket()
{
}

const std::string ssl_socket::service_protocol() const
{
    return "443";
}

//SSL通信のコネクション確立(TCPレイヤーでコネクションを行う)
ssl_socket::error_code& ssl_socket::connect(endpoint_type& begin, error_code& ec)
{
    ec = socket_.lowest_layer().connect(begin, ec);
    return ec;
}
void ssl_socket::async_connect(endpoint_type& begin, ConnectHandler handler)
{
    socket_.lowest_layer().async_connect(begin, handler);
    return;
}
    
void ssl_socket::handshake(handshake_type type)
{
    socket_.handshake(type);
    return;
}
void ssl_socket::async_handshake(handshake_type type, HandshakeHandler handler)
{
    socket_.async_handshake(type, handler);
    return;
}
    
//[(SSLレイヤーの処理 ->] TCPレイヤーの処理
void ssl_socket::close()
{
    socket_.lowest_layer().close();
    return;
}
void ssl_socket::shutdown(shutdown_type what)
{
    socket_.shutdown();
    socket_.lowest_layer().shutdown(what);
    return;
}

} // namespace application_layer
} // namespace bstcon

#endif
#endif
