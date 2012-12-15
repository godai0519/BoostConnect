//
// sync_connection.hpp
// ~~~~~~~~~~
//
// 同期のためのクラス群
//

#ifndef BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_HPP
#define BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "connection_base.hpp"

namespace bstcon{
namespace connection_type{

class sync_connection : public connection_common<sync_connection>{
public:
    sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket);
    virtual ~sync_connection();
    
    connection_ptr connect(const std::string& host,ConnectionHandler handler);
    connection_ptr connect(const endpoint_type& ep,ConnectionHandler handler);

    response_type send(boost::shared_ptr<boost::asio::streambuf> buf, EndHandler end_handler, ChunkHandler chunk_handler);

private:
    void handle_read(const error_code& ec, EndHandler end_handler);
};

} // namespace connection_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/sync_connection.ipp"
#endif

#endif
