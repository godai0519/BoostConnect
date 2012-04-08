//
// ssl_socket.hpp
// ~~~~~~~~~~
//
// SSL通信で特別に行われることを行う
//

#ifndef NO_SSL
#ifndef BOOSTCONNECT_APPLAYER_SSL_SOCKET
#define BOOSTCONNECT_APPLAYER_SSL_SOCKET

#include <boost/asio.hpp>
#include "socket_base.hpp"

namespace bstcon{
namespace application_layer{
  
class ssl_socket : public socket_common<socket_base::ssl_socket_type>{
  typedef socket_common<socket_base::ssl_socket_type> my_base;

public:
  ssl_socket(io_service& io_service,context_type& ctx) : my_base(io_service,ctx){}
  virtual ~ssl_socket(){}

  const std::string service_protocol() const { return "https"; }

  //SSL通信のコネクション確立(TCPレイヤーでコネクションを行う)
  error_code& connect(endpoint_type& begin,error_code& ec)
  {
    ec = socket_.lowest_layer().connect(begin,ec);
    return ec;
  }
  void async_connect(endpoint_type& begin,ConnectHandler handler)
  {
    socket_.lowest_layer().async_connect(begin,handler);
    return;
  }
  
  void handshake(handshake_type type)
  {
    socket_.handshake(type);
    return;
  }
  void async_handshake(handshake_type type,HandshakeHandler handler)
  {
    socket_.async_handshake(type,handler);
    return;
  }
  
  //[(SSLレイヤーの処理 ->] TCPレイヤーの処理
  void close()
  {
    socket_.lowest_layer().close();
    return;
  }
  void shutdown(shutdown_type what)
  {
    socket_.shutdown();
    socket_.lowest_layer().shutdown(what);
    return;
  }
};

} // namespace application_layer
} // namespace bstcon

#endif
#endif
