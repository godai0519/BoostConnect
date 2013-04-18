//
// async_connection.hpp
// ~~~~~~~~~~
//
// Specialization from connection_base to ASync Connection
//

#ifndef BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION_HPP
#define BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "connection_base.hpp"

namespace bstcon{
namespace connection_type{

class async_connection : public connection_common<async_connection>{
public:
    async_connection(const boost::shared_ptr<application_layer::socket_base>& socket);
    virtual ~async_connection();

    void close();
    
    connection_ptr connect(const std::string& host,ConnectionHandler handler);
    connection_ptr connect(const endpoint_type& ep,ConnectionHandler handler);

    std::future<response_type> send(boost::shared_ptr<boost::asio::streambuf> buf, EndHandler end_handler, ChunkHandler chunk_handler);

private:
    boost::scoped_ptr<boost::asio::ip::tcp::resolver> resolver_;
    EndHandler end_handler_;

    void handle_resolve(boost::asio::ip::tcp::resolver::iterator ep_iterator, const boost::system::error_code& ec, ConnectionHandler handler);

    void handle_connect(const boost::system::error_code& ec, ConnectionHandler handler);
    void handle_write(const boost::shared_ptr<std::promise<response_type>> p, boost::shared_ptr<boost::asio::streambuf> buf, const boost::system::error_code& ec, ChunkHandler chunk_handler);

    void handle_read(const boost::shared_ptr<std::promise<response_type>> p, const error_code& ec);
};

} // namespace connection_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/async_connection.ipp"
#endif

#endif
