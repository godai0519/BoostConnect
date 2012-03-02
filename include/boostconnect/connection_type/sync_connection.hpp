//
// async_connection.hpp
// ~~~~~~~~~~
//
// 非同期のためのクラス群
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
	
	//追加	
	void operator() (
		const std::string& host,
		boost::asio::streambuf& buf,
		//error_code& ec,
		ReadHandler handler
		)
	{
		if(busy)
		{
			throw std::runtime_error("SOCKET_BUSY");
			//例外！！
		}
		busy = true;
		handler_ = handler;

		boost::asio::ip::tcp::resolver resolver(socket_layer_->get_io_service());
		boost::asio::ip::tcp::resolver::query query(host,socket_layer_->service_protocol());
		boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query);
		
		socket_layer_->connect(ep_iterator); //resolve完了しているならその名前解決先に接続(ハンドシェイク)
		socket_layer_->write(buf); //書き込み
		socket_layer_->read(boost::bind(&sync_connection::handle_read,this,boost::asio::placeholders::error)); //読み込み
		
		busy = false;

		return;
	}
private:
	void handle_read(const error_code& ec)
	{
		//std::cout << "SYNC";
		handler_(ec);
		//std::cout << "END";
	}
	
	ReadHandler handler_;
	bool busy;
	/*
	//追加	
	error_code& operator() (
		boost::asio::ip::tcp::resolver::iterator ep_iterator,
		boost::asio::streambuf& buf,
		error_code& ec
		)
	{
		
		//非同期実装
		return ec;
	}*/
};

} // namespace connection_type
} // namespace protocol
} // namespace oauth

#endif
