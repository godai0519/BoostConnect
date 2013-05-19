//
// sync_connection.ipp
// ~~~~~~~~~~
//
// Specialization from connection_base to Sync Connection
//

#ifndef BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_IPP
#define BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_IPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../sync_connection.hpp"

namespace bstcon{
namespace connection_type{

sync_connection::sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
{
    socket_ = socket;
}

sync_connection::~sync_connection()
{
}
    
sync_connection::connection_ptr sync_connection::connect(const std::string& host, ConnectionHandler handler)
{
    host_ = host;
    boost::asio::ip::tcp::resolver resolver(socket_->get_io_service());
    boost::asio::ip::tcp::resolver::query query(host, socket_->service_protocol());
        
    // Connect Start
    error_code ec;
    boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query, ec);
    boost::asio::connect(socket_->lowest_layer(), ep_iterator, ec);

#ifdef USE_SSL_BOOSTCONNECT
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#else
    socket_->handshake();
#endif

    handler(shared_from_this(), ec);
    return this->shared_from_this();
}

sync_connection::connection_ptr sync_connection::connect(const endpoint_type& ep, ConnectionHandler handler)
{
    host_ = ep.address().to_string();

    // Connect Start
    error_code ec;
    socket_->lowest_layer().connect(ep, ec);
#ifdef USE_SSL_BOOSTCONNECT
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#else
    socket_->handshake();
#endif

    handler(shared_from_this(), ec);
    return this->shared_from_this();
}

auto sync_connection::send(boost::shared_ptr<boost::asio::streambuf> buf, EndHandler end_handler, ChunkHandler chunk_handler) -> std::future<response_type>
{
    const auto p = boost::make_shared<std::promise<response_type>>();

    // TODO: 要検討コード
    reader_.reset(new reader());
    // TODO END

    error_code ec;
    boost::asio::write(*socket_, *buf, ec);

    if(!ec)
    {
        reader_->read_starter(
            *socket_,
            boost::bind(&sync_connection::handle_read, shared_from_this(), p, boost::asio::placeholders::error, end_handler),
            chunk_handler);
    }

    return p->get_future();
}

void sync_connection::handle_read(const boost::shared_ptr<std::promise<response_type>> p, const error_code& ec, EndHandler end_handler)
{
    // Event notification to std::future made by std::promise
    end_handler(reader_->get_response(), ec);
    p->set_value(reader_->get_response());
    reader_.reset();
    return;
}

} // namespace connection_type
} // namespace bstcon

#endif
