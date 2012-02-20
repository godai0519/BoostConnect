//
// async_connection.hpp
// ~~~~~~~~~~
//
// ”ñ“¯Šú‚Ì‚½‚ß‚ÌƒNƒ‰ƒXŒQ
//

#ifndef TWIT_LIB_PROTOCOL_CONNECTTYPE_SYNC_CONNECTION
#define TWIT_LIB_PROTOCOL_CONNECTTYPE_SYNC_CONNECTION

#include <memory>
#include <boost/asio.hpp>
#include "connection_base.hpp"
#include "../application_layer/layer_base.hpp"

namespace oauth{
namespace protocol{
namespace connection_type{

class sync_connection : public connection_base{
public:
	sync_connection(std::shared_ptr<application_layer::layer_base>& socket_layer)
	{
		socket_layer_ = socket_layer;
		busy = false;
	}
	virtual ~sync_connection(){}
	
	//’Ç‰Á	
	error_code& operator() (
		const std::string& host,
		boost::asio::streambuf& buf,
		error_code& ec,
		ReadHandler handler
		)
	{
		if(busy)
		{
			std::cout << "\n\nTHIS CLIENT is busy\n\n";
			return ec;
		}
		busy = true;
		handler_ = handler;

		boost::asio::ip::tcp::resolver resolver(socket_layer_->get_io_service());
		boost::asio::ip::tcp::resolver::query query(host,socket_layer_->service_protocol());
		boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query,ec);
		if(ec) return ec;

		socket_layer_->connect(ep_iterator,ec);
		if(ec) return ec;

		socket_layer_->write(buf,ec);
		if(ec) return ec;

		socket_layer_->read(ec,boost::bind(&sync_connection::handle_read,this,boost::asio::placeholders::error));
		
		busy = false;

		return ec;
	}
private:
	void handle_read(const error_code& ec)
	{
		std::cout << "SYNC";
		handler_(ec);
		std::cout << "END";
	}
	
	ReadHandler handler_;
	bool busy;
	/*
	//’Ç‰Á	
	error_code& operator() (
		boost::asio::ip::tcp::resolver::iterator ep_iterator,
		boost::asio::streambuf& buf,
		error_code& ec
		)
	{
		
		//”ñ“¯ŠúŽÀ‘•
		return ec;
	}*/
};

} // namespace connection_type
} // namespace protocol
} // namespace oauth

#endif
