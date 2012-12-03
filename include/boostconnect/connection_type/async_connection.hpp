//
// async_connection.hpp
// ~~~~~~~~~~
//
// 非同期のためのクラス群
//

#ifndef BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION
#define BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION

#include <memory>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include "connection_base.hpp"
#include "../system/error_code.hpp"

namespace bstcon{
namespace connection_type{

class async_connection : public connection_common<async_connection>{
public:
    async_connection(const boost::shared_ptr<application_layer::socket_base>& socket);
    virtual ~async_connection();

    void close();
    
    connection_ptr connect(const std::string& host,ConnectionHandler handler);
    connection_ptr connect(const endpoint_type& ep,ConnectionHandler handler);

    response_type send(boost::shared_ptr<boost::asio::streambuf> buf, EndHandler end_handler, ChunkHandler chunk_handler);

private:
    boost::scoped_ptr<boost::asio::ip::tcp::resolver> resolver_;
    boost::shared_ptr<boost::asio::streambuf> buf_;

    void handle_resolve(boost::asio::ip::tcp::resolver::iterator ep_iterator, const boost::system::error_code& ec, ConnectionHandler handler);

    void handle_connect(const boost::system::error_code& ec, ConnectionHandler handler);
    void handle_write(const boost::system::error_code& ec, EndHandler end_handler, ChunkHandler chunk_handler);

    void handle_read(const error_code& ec, EndHandler end_handler);
};

} // namespace connection_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "../../../src/connection_type/async_connection.cpp"
#endif

#endif
