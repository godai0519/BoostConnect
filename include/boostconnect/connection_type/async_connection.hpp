//
// async_connection.hpp
// ~~~~~~~~~~
//
// 非同期のためのクラス群
//


#ifndef TWIT_LIB_PROTOCOL_CONNECTTYPE_ASYNC_CONNECTION
#define TWIT_LIB_PROTOCOL_CONNECTTYPE_ASYNC_CONNECTION

#include <memory>
#include <boost/asio.hpp>
#include "connection_base.hpp"
#include "../application_layer/layer_base.hpp"

namespace oauth{
namespace protocol{
namespace connection_type{

class async_connection : public connection_base{
public:
	async_connection(std::shared_ptr<application_layer::layer_base>& socket_layer)
	{
		socket_layer_ = socket_layer;
		resolver_ = new boost::asio::ip::tcp::resolver(socket_layer_->get_io_service());
		busy = false;
	}
	virtual ~async_connection(){}
	
	//追加	
	void operator() (
		const std::string& host,
		boost::asio::streambuf& buf,
		ReadHandler handler
		)
	{
		error_code ec;
		if(busy)
		{
			std::cout << "\n\nTHIS CLIENT is busy\n\n";
			//例外！！
			//(未実装)
		}
		busy = true;

		buf_ = &buf;
		handler_ = handler;
		//resolver_ = std::shared_ptr<boost::asio::ip::tcp::resolver>(new boost::asio::ip::tcp::resolver(socket_layer_->get_io_service()));
		//boost::asio::ip::tcp::resolver resolver(socket_layer_->get_io_service());
		//query_ = std::shared_ptr<boost::asio::ip::tcp::resolver::query>(new boost::asio::ip::tcp::resolver::query(host,socket_layer_->service_protocol()));
		boost::asio::ip::tcp::resolver::query query(host,socket_layer_->service_protocol());
		resolver_->async_resolve(query,
			boost::bind(&async_connection::handle_resolve,this,
			boost::asio::placeholders::iterator,
			boost::asio::placeholders::error)); //handle_resolveへ

		//非同期実装
		//return ec;
	}
	/*
	//追加	
	error_code& operator() (
		boost::asio::ip::tcp::resolver::iterator ep_iterator,
		boost::asio::streambuf& buf,
		error_code& ec
		)
	{
		buf_ = &buf;
		socket_layer_->async_connect(ep_iterator,
			boost::bind(&async_connection::handle_connect,this,
				boost::asio::placeholders::error));
		
		//非同期実装
		return ec;
	}*/

private:
	ReadHandler handler_;
	bool busy;

	boost::asio::ip::tcp::resolver* resolver_;
	boost::asio::streambuf *buf_;
	void handle_resolve(boost::asio::ip::tcp::resolver::iterator ep_iterator,const boost::system::error_code& ec)
	{
		if(!ec)
		{
			socket_layer_->async_connect(ep_iterator,
				boost::bind(&async_connection::handle_connect,this,
					boost::asio::placeholders::error));
		}
		else std::cout << "Error Resolve!?\n" << ec.message() << std::endl;
	}
	void handle_connect(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			socket_layer_->async_write(*buf_,
				boost::bind(&async_connection::handle_write,this,
					boost::asio::placeholders::error));
		}
		else std::cout << "Error Connect or HandShake!?" << std::endl;
	}
	void handle_write(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			socket_layer_->async_read(
				boost::bind(&async_connection::handle_read,this,boost::asio::placeholders::error)
				);
		}
		else std::cout << "Error Write!?" << std::endl;
	}
	void handle_read(const error_code& ec)
	{
		std::cout << "ASYNC";
		handler_(ec);
		std::cout << "END";
		busy = false;
	}
};

} // namespace connection_type
} // namespace protocol
} // namespace oauth

#endif
