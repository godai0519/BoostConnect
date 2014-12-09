//
// tcp_socket.hpp
// ~~~~~~~~~~
//
// Specialization from socket_base to TCP connection
//

#ifndef BOOSTCONNECT_APPLAYER_TCP_SOCKET_HPP
#define BOOSTCONNECT_APPLAYER_TCP_SOCKET_HPP

#include <boost/asio.hpp>
#include <boostconnect/application_layer/socket_base.hpp>

namespace bstcon{
namespace application_layer{

class tcp_socket : public socket_common<socket_base::tcp_socket_type>
{
public:
    tcp_socket(io_service& io_service);
    virtual ~tcp_socket();

    std::string service_protocol() const;

    //TCP通信のコネクション確立
    error_code& connect(endpoint_type& begin, error_code& ec);
    void async_connect(endpoint_type& begin, ConnectHandler handler);

#ifdef USE_SSL_BOOSTCONNECT
    //TCP通信ではHandshakeを行わない -> Handlerを直接呼び出す
    void handshake(handshake_type);
    void async_handshake(handshake_type, HandshakeHandler handler);
#endif

    //TCPレイヤーの処理
    void close();
    void shutdown(shutdown_type const what);
};

} // namespace application_layer
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/tcp_socket.ipp"
#endif

#endif
