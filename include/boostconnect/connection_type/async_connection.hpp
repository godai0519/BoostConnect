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
//#include "../application_layer/layer_base.hpp"

namespace oauth{
namespace protocol{
namespace connection_type{

class async_connection : public connection_base{
public:
	async_connection(const std::shared_ptr<application_layer::socket_base>& socket/*std::shared_ptr<application_layer::layer_base>& socket_layer*/)
	{
		socket_ = socket;
		//socket_layer_ = socket_layer;
		resolver_ = new boost::asio::ip::tcp::resolver(socket_->get_io_service());
		busy = false;
	}
	virtual ~async_connection(){}
	
	//追加	
	response_type operator() (
		const std::string& host,
		boost::asio::streambuf& buf,
		ReadHandler handler
		)
	{
		error_code ec;
		if(busy)
		{
			throw std::runtime_error("SOCKET_BUSY");
			//例外！！
		}
		busy = true;

		buf_ = &buf;
		handler_ = handler;
		
		reader_.reset(new reader());

		boost::asio::ip::tcp::resolver::query query(host,socket_->service_protocol());
		resolver_->async_resolve(query,
			boost::bind(&async_connection::handle_resolve,this,
			boost::asio::placeholders::iterator,
			boost::asio::placeholders::error)); //handle_resolveへ

		//非同期実装
		//return ec;
		return reader_->get_response();
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
			boost::asio::async_connect(socket_->lowest_layer(),ep_iterator,
				boost::bind(&async_connection::handle_connect,this,
					boost::asio::placeholders::error));
		}
		else std::cout << "Error Resolve!?\n" << ec.message() << std::endl;
	}
	void handle_connect(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			socket_->async_handshake(application_layer::socket_base::ssl_socket_type::client,
				boost::bind(&async_connection::handle_handshake,this,
					boost::asio::placeholders::error));
		}
		else std::cout << "Error Connect!?" << std::endl;
	}
	void handle_handshake(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			boost::asio::async_write(*socket_.get(),*buf_,
				boost::bind(&async_connection::handle_write,this,
					boost::asio::placeholders::error));
		}
		else std::cout << "Error HandShake!?\n" << ec.message() << std::endl;
	}
	void handle_write(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			reader_->async_read_starter(*socket_.get(),boost::bind(&async_connection::handle_read,this,boost::asio::placeholders::error));
			//socket_layer_->async_read(
			//	boost::bind(&async_connection::handle_read,this,boost::asio::placeholders::error)
			//	);
		}
		else std::cout << "Error Write!?" << std::endl;
	}
	void handle_read(const error_code& ec)
	{
		//std::cout << "ASYNC";
		handler_(ec);
		//std::cout << "END";
		busy = false;
	}
};

} // namespace connection_type
} // namespace protocol
} // namespace oauth

#endif
