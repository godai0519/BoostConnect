//
// sync_connection.hpp
// ~~~~~~~~~~
//
// Specialization from connection_base to Sync Connection
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
    explicit sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket);
    virtual ~sync_connection();

	connection_ptr connect(const std::string&, ConnectionHandler);
	connection_ptr connect(const endpoint_type&, ConnectionHandler);

	std::future<std::string> read(ReadHandler handler = ReadHandler());
	std::future<std::string> read_size(const std::size_t size, ReadHandler handler = ReadHandler());
	std::future<std::string> read_until(const std::string& until, ReadHandler handler = ReadHandler());

	std::future<std::size_t> write(const boost::shared_ptr<boost::asio::streambuf>& buf, WriteHandler handler = WriteHandler());

private:
	boost::shared_ptr<boost::asio::streambuf> read_buf_;
};

} // namespace connection_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/sync_connection.ipp"
#endif

#endif
