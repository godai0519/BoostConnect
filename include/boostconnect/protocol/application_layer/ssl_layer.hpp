//
// ssl_layer.hpp
// ~~~~~~~~~~
//
// SSL通信で特別に行われることを行う
//

#ifndef NO_SSL //SSL使わない設定なら読み込みさえもしない
#ifndef TWIT_LIB_PROTOCOL_APPLAYER_SSL_LAYER
#define TWIT_LIB_PROTOCOL_APPLAYER_SSL_LAYER

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include "layer_base.hpp"
#include "../response_reader/response_container.hpp"

namespace oauth{
namespace protocol{
namespace application_layer{

class ssl_layer : public layer_base{
public:
	ssl_layer(io_service &io_service,context &ctx) : socket_(io_service,ctx){}
	virtual ~ssl_layer(){}
	
	io_service& get_io_service(){return socket_.get_io_service();}
	const std::string service_protocol() const {return std::string("https");}
	
	error_code& connect(boost::asio::ip::tcp::resolver::iterator& ep_iterator,error_code& ec)
	{
		boost::asio::connect(socket_.lowest_layer(),ep_iterator,ec);
		if(!ec) socket_.handshake(ssl_socket::client,ec);
		
		//connectできてたらhandshakeの結果
		//できてなかったらhandshakeせずに返す
		return ec; 
	}
	error_code& write(boost::asio::streambuf& buf,error_code& ec)
	{
		boost::asio::write(socket_,buf,ec);
		return ec;
	}
	
	error_code& read(error_code& ec)
	{
		return response_.read_starter(socket_,ec);
	}
	void async_connect(boost::asio::ip::tcp::resolver::iterator& ep_iterator,ConnectHandler handler)
	{
		boost::asio::async_connect(socket_.lowest_layer(),
			ep_iterator,
			boost::bind(&ssl_layer::async_handshake,this,
				handler,
				boost::asio::placeholders::error));
	}
	void async_write(boost::asio::streambuf& buf,WriteHandler handler)
	{
		boost::asio::async_write(socket_,
			buf,
			boost::bind(handler,
				boost::asio::placeholders::error));
	}
	void async_read()
	{
		response_.async_read_starter(socket_);
	}
	/*void async_read_header(boost::asio::streambuf& buf,ReadHandler handler)
	{
		boost::asio::async_read_until(socket_,
			buf,
			"\r\n\r\n",
			boost::bind(handler,
				boost::asio::placeholders::error));
	}
	void async_read_body(boost::asio::streambuf&,ReadHandler)
	{
		//ヘッダーの読み込み
		socket_layer_->response_.read_head(*response_buf_);



	}*/

private:
	//connectが成立すればここでhandshake
	void async_handshake(ConnectHandler handler,const error_code& ec)
	{
		if(!ec)
		{
			socket_.async_handshake(ssl_socket::client,handler);
		}
		else std::cout << "Error Connect!?" << std::endl;
	}
	ssl_socket socket_;
};

} // namespace application_layer
} // namespace protocol
} // namespace oauth

#endif
#endif
