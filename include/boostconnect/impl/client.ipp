//
// client.ipp
// ~~~~~~~~~~
//
// Main Client Connection provide class
//

#ifndef BOOSTCONNECT_CLIENT_IPP
#define BOOSTCONNECT_CLIENT_IPP

#include "../client.hpp"

namespace bstcon{

//TCP
client::client(io_service &io_service,const connection_type::connection_type& connection_type)
    : connection_type_(connection_type), io_service_(io_service)
#ifdef USE_SSL_BOOSTCONNECT
    , ctx_(nullptr)
#endif
{
}
    
#ifdef USE_SSL_BOOSTCONNECT
//SSL
typedef boost::asio::ssl::context context;
client::client(io_service &io_service,context &ctx,const connection_type::connection_type& connection_type)
    : io_service_(io_service), connection_type_(connection_type), ctx_(&ctx)
{
}
#endif
    
template<typename T>
client::connection_ptr client::operator() (
    const T& host,
    ConnectionHandler handler
    )
{
    auto connection = crerate_connection();
    connection->connect(host, handler);

    return connection;
}

template client::connection_ptr client::operator()<std::string> (const std::string& host, ConnectionHandler handler);
template client::connection_ptr client::operator()<client::endpoint_type> (const endpoint_type& host, ConnectionHandler handler);

const std::string client::service_protocol() const
{
#ifdef USE_SSL_BOOSTCONNECT
    return (ctx_==nullptr) ? "http" : "https";
#endif
    return "http";
}

void client::set_connection_type(const connection_type::connection_type& connection_type)
{
    connection_type_ = connection_type;
}

inline client::socket_ptr client::create_socket()
{
    socket_ptr socket;

#ifdef USE_SSL_BOOSTCONNECT
if(ctx_ != nullptr)
    socket.reset(new bstcon::application_layer::ssl_socket(io_service_,*ctx_));
else
#endif
    socket.reset(new bstcon::application_layer::tcp_socket(io_service_));

    return socket;
}
inline const client::connection_ptr client::crerate_connection()
{
    connection_ptr connection;

    if(connection_type_ == connection_type::sync)
        connection.reset(new bstcon::connection_type::sync_connection(create_socket()));
    else 
        connection.reset(new bstcon::connection_type::async_connection(create_socket()));

    return connection;
}

} // namespace bstcon

#endif
