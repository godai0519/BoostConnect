#include <boostconnect/application_layer/socket_base.hpp>

namespace bstcon{
namespace application_layer{
    
//こいつらは無視の方針で(というかインターフェース)
socket_base::socket_base(){}
socket_base::~socket_base(){}

template<class Socket>
socket_common<Socket>::socket_common(io_service& io_service) : socket_(io_service)
{
}

#ifdef USE_SSL_BOOSTCONNECT
template<class Socket>
socket_common<Socket>::socket_common(io_service& io_service,context_type& ctx) : socket_(io_service,ctx)
{
}
#endif

template<class Socket>
socket_common<Socket>::~socket_common(){}

template<class Socket>
typename socket_common<Socket>::lowest_layer_type& socket_common<Socket>::lowest_layer()
{
    return socket_.lowest_layer();
}
template<class Socket>
typename socket_common<Socket>::io_service& socket_common<Socket>::get_io_service()
{
    return socket_.get_io_service();
}

template<class Socket>
std::size_t socket_common<Socket>::read_some(const mutable_buffer& buf,error_code& ec)
{
    return socket_.read_some(buf,ec);
}
template<class Socket>
std::size_t socket_common<Socket>::write_some(const const_buffer& buf,error_code& ec)
{
    return socket_.write_some(buf,ec);
}
template<class Socket>
std::size_t socket_common<Socket>::write_some(const consuming_buffer& buf,error_code& ec)
{
    return socket_.write_some(buf,ec);
}
template<class Socket>
void socket_common<Socket>::async_read_some(const mutable_buffer& buf,ReadHandler handler)
{
    socket_.async_read_some(buf,handler);
    return;
}
template<class Socket>
void socket_common<Socket>::async_write_some(const const_buffer& buf,WriteHandler handler)
{
    socket_.async_write_some(buf,handler);
    return;
}
template<class Socket>
void socket_common<Socket>::async_write_some(const consuming_buffer& buf,WriteHandler handler)
{
    socket_.async_write_some(buf,handler);
    return;
}

} // namespace application_layer
} // namespace bstcon

#ifndef BOOSTCONNECT_LIB_BUILD
#include <boostconnect/application_layer/tcp_socket.hpp>
#include <boostconnect/application_layer/ssl_socket.hpp>
#endif
