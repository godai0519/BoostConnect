#ifndef BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_IPP
#define BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_IPP

#include <boostconnect/connection_type/sync_connection.hpp>

namespace bstcon{
namespace connection_type{

sync_connection::sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
{
    socket_ = socket;
    reader_.reset(new reader());
}
sync_connection::~sync_connection()
{
}
    
sync_connection::connection_ptr sync_connection::connect(const std::string& host,ConnectionHandler handler)
{
    boost::asio::ip::tcp::resolver resolver(socket_->get_io_service());
    boost::asio::ip::tcp::resolver::query query(host,socket_->service_protocol());
        
    // Connect Start
    error_code ec;
    boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query,ec);
    boost::asio::connect(socket_->lowest_layer(),ep_iterator,ec);

#ifdef USE_SSL_BOOSTCONNECT
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#else
    socket_->handshake();
#endif

    handler(shared_from_this(), ec);
    return this->shared_from_this();
}

sync_connection::connection_ptr sync_connection::connect(const endpoint_type& ep,ConnectionHandler handler)
{
    // Connect Start
    error_code ec;
    socket_->lowest_layer().connect(ep,ec);
#ifdef USE_SSL_BOOSTCONNECT
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#else
    socket_->handshake();
#endif

    handler(shared_from_this(), ec);
    return this->shared_from_this();
}

sync_connection::response_type sync_connection::send(boost::shared_ptr<boost::asio::streambuf> buf, EndHandler end_handler, ChunkHandler chunk_handler)
{
    reader_.reset(new connection_base::reader());
    response_type response = reader_->get_response();

    error_code ec;
    boost::asio::write(*socket_, *buf, ec);

    if(!ec)
    {
        reader_->read_starter(
            *socket_,
            boost::bind(&sync_connection::handle_read, shared_from_this(), boost::asio::placeholders::error, end_handler),
            chunk_handler);
    }
    return response;
}

void sync_connection::handle_read(const error_code& ec, EndHandler end_handler)
{
    auto response = reader_->get_response();
    reader_.reset();

    end_handler(response, ec);
    return;
}

} // namespace connection_type
} // namespace bstcon

#endif
