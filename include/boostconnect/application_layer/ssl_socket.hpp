//
// ssl_socket.hpp
// ~~~~~~~~~~
//
// SSL通信で特別に行われることを行う
//

#ifdef USE_SSL_BOOSTCONNECT
#ifndef BOOSTCONNECT_APPLAYER_SSL_SOCKET
#define BOOSTCONNECT_APPLAYER_SSL_SOCKET

#include <boost/asio.hpp>
#include "socket_base.hpp"

namespace bstcon{
namespace application_layer{
    
class ssl_socket : public socket_common<socket_base::ssl_socket_type>
{
    typedef socket_common<socket_base::ssl_socket_type> my_base;

public:
    ssl_socket(io_service& io_service,context_type& ctx);
    virtual ~ssl_socket();

    const std::string service_protocol() const;

    //SSL通信のコネクション確立(TCPレイヤーでコネクションを行う)
    error_code& connect(endpoint_type& begin,error_code& ec);
    void async_connect(endpoint_type& begin,ConnectHandler handler);
    
    void handshake(handshake_type type);
    void async_handshake(handshake_type type,HandshakeHandler handler);
    
    //[(SSLレイヤーの処理 ->] TCPレイヤーの処理
    void close();
    void shutdown(shutdown_type what);
};

} // namespace application_layer
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "../../../src/application_layer/ssl_socket.cpp"
#endif

#endif
#endif
