//
// async_connection.ipp
// ~~~~~~~~~~
//
// Specialization from connection_base to ASync Connection
//

#ifndef BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION_IPP
#define BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION_IPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../async_connection.hpp"

namespace bstcon{
namespace connection_type{

async_connection::async_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
{
    socket_ = socket;
    resolver_.reset(new boost::asio::ip::tcp::resolver(socket_->get_io_service()));
}
async_connection::~async_connection()
{
}

void async_connection::close()
{
    socket_->close();
    return;
}

async_connection::connection_ptr async_connection::connect(const std::string& host,ConnectionHandler handler)
{
    boost::asio::ip::tcp::resolver::query query(host, socket_->service_protocol());
    resolver_->async_resolve(
        query,
        boost::bind(&async_connection::handle_resolve, shared_from_this(), boost::asio::placeholders::iterator, boost::asio::placeholders::error, handler)
        );

    return this->shared_from_this();
}
async_connection::connection_ptr async_connection::connect(const endpoint_type& ep,ConnectionHandler handler)
{
    socket_->lowest_layer().async_connect(
        ep,
        boost::bind(handler, shared_from_this(), boost::asio::placeholders::error)
        );

    return this->shared_from_this();
}

auto async_connection::send(boost::shared_ptr<boost::asio::streambuf> buf, EndHandler end_handler, ChunkHandler chunk_handler) -> std::future<response_type>
{
    end_handler_ = end_handler;

    const auto p = boost::make_shared<std::promise<response_type>>();
    reader_.reset(new reader());

    boost::asio::async_write(*socket_, *buf,
        boost::bind(&async_connection::handle_write, shared_from_this(), p, boost::asio::placeholders::error, chunk_handler));

    return p->get_future();
}

void async_connection::handle_resolve(boost::asio::ip::tcp::resolver::iterator ep_iterator, const boost::system::error_code& ec, ConnectionHandler handler)
{
    if(!ec)
    {
        boost::asio::async_connect(
            socket_->lowest_layer(),
            ep_iterator,
            boost::bind(&async_connection::handle_connect, shared_from_this(), boost::asio::placeholders::error, handler)
            );
    }
    else std::cout << "Error Resolve!?\n" << ec.message() << std::endl;
}

void async_connection::handle_connect(const boost::system::error_code& ec, ConnectionHandler handler)
{
    if(!ec)
    {
#ifdef USE_SSL_BOOSTCONNECT
        socket_->async_handshake(
            application_layer::socket_base::ssl_socket_type::client,
            boost::bind(handler, shared_from_this(), boost::asio::placeholders::error)
            );
#else
        socket_->async_handshake(
            boost::bind(handler, shared_from_this(), boost::asio::placeholders::error)
            );
#endif
    }
    else std::cout << "Error Connect!?" << std::endl;
}

void async_connection::handle_write(const boost::shared_ptr<std::promise<response_type>> p, const boost::system::error_code& ec, ChunkHandler chunk_handler)
{
    buf_.reset();
    if(!ec)
    {
        reader_->async_read_starter(
            *socket_,
            boost::bind(&async_connection::handle_read, shared_from_this(), p, boost::asio::placeholders::error),
            chunk_handler
            );
    }
    else std::cout << "Error Write!?" << ec.message() << std::endl;
}

void async_connection::handle_read(const boost::shared_ptr<std::promise<response_type>> p, const error_code& ec)
{
    // Event notification to std::future made by std::promise
    end_handler_(reader_->get_response(), ec);
    p->set_value(reader_->get_response());
    reader_.reset();
    return;
}

} // namespace connection_type
} // namespace bstcon

#endif
