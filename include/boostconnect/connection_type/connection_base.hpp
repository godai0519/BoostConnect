//
// connection_base.hpp
// ~~~~~~~~~~
//
// Provide Sync/ASync Connection Common Class powered by Boost.Asio
//

#ifndef BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE_HPP
#define BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE_HPP

#include <future>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional/optional.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../application_layer/socket_base.hpp"
#include "../request.hpp"
#include "../response.hpp"

namespace bstcon{
namespace connection_type{
	
class connection_base : boost::noncopyable{
public:
    typedef boost::system::error_code           error_code;
    typedef boost::asio::ip::tcp::endpoint      endpoint_type;
    typedef bstcon::request                     request_type;

    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

	typedef boost::function<void(connection_ptr, error_code)> ConnectionHandler;	
    typedef boost::function<void(const std::size_t)> WriteHandler;
    typedef boost::function<void(const std::string&)> ReadHandler;

    connection_base() = default;
    virtual ~connection_base() = default;

    virtual connection_ptr connect(const std::string&, ConnectionHandler) = 0;
    virtual connection_ptr connect(const endpoint_type&, ConnectionHandler) = 0;
    
	inline virtual void close()
	{
		socket_->close();
		return;
	}

	virtual std::future<std::string> read(ReadHandler handler = ReadHandler()) = 0;
	virtual std::future<std::string> read_size(const std::size_t size, ReadHandler handler = ReadHandler()) = 0;
	virtual std::future<std::string> read_until(const std::string& until, ReadHandler handler = ReadHandler()) = 0;

	inline virtual std::future<std::string> read_line(ReadHandler handler = ReadHandler())
	{
		return read_until("\n", handler);
	}

	virtual std::future<std::size_t> write(const boost::shared_ptr<boost::asio::streambuf>& buf, WriteHandler handler = WriteHandler()) = 0;

protected:
    boost::shared_ptr<application_layer::socket_base> socket_;
};

template <class Devide>
class connection_common : public connection_base, public boost::enable_shared_from_this<Devide>{};

} // namespace connection_type
} // namespace bstcon

#endif
