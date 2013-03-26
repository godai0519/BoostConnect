//
// tcp_socket.ipp
// ~~~~~~~~~~
//
// tcp_socketの実装
//

#ifndef BOOSTCONNECT_APPLAYER_TCP_SOCKET_IPP
#define BOOSTCONNECT_APPLAYER_TCP_SOCKET_IPP

#include <boost/asio.hpp>
#include "../tcp_socket.hpp"

namespace bstcon{
namespace application_layer{
    
tcp_socket::tcp_socket(io_service& io_service) : my_base(io_service)
{
}
tcp_socket::~tcp_socket(){}
   
const std::string tcp_socket::service_protocol() const
{
    return "80";
}

//TCP通信のコネクション確立
tcp_socket::error_code& tcp_socket::connect(endpoint_type& begin,error_code& ec)
{
    ec = socket_.connect(begin,ec);
    return ec;
}
void tcp_socket::async_connect(endpoint_type& begin,ConnectHandler handler)
{
    socket_.async_connect(begin,handler);
    return;
}
    
#ifdef USE_SSL_BOOSTCONNECT
//TCP通信ではHandshakeを行わない -> Handlerを直接呼び出す
void tcp_socket::handshake(handshake_type)
{
    return;
}
void tcp_socket::async_handshake(handshake_type,HandshakeHandler handler)
{
    handler(error_code());
    return;
}
#else
void tcp_socket::handshake()
{
    return;
}
void tcp_socket::async_handshake(HandshakeHandler handler)
{
    handler(error_code());
    return;
}
#endif

//TCPレイヤーの処理
void tcp_socket::close()
{
    socket_.close();
    return;
}
void tcp_socket::shutdown(shutdown_type what)
{
    socket_.shutdown(what);
    return;
}

} // namespace application_layer
} // namespace bstcon

#endif
