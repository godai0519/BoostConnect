//
// tcp_layer.hpp
// ~~~~~~~~~~
//
// TCP通信で特別に行われることを行う
//

#ifndef TWIT_LIB_PROTOCOL_APPLAYER_TCP_LAYER
#define TWIT_LIB_PROTOCOL_APPLAYER_TCP_LAYER

#include "layer_base.hpp"

namespace oauth{
namespace protocol{
namespace application_layer{

//ここで基底クラスの実装を行うが。
class tcp_layer : public layer_base{
public:
	tcp_layer(io_service &io_service) : socket_(io_service){}
	virtual ~tcp_layer(){}
	
	io_service& get_io_service(){return socket_.get_io_service();}
	const std::string service_protocol() const {return std::string("http");}
	void close(){socket_.close();}

	void connect(boost::asio::ip::tcp::resolver::iterator& ep_iterator)
	{
		boost::asio::connect(socket_,ep_iterator);
	}
	void write(boost::asio::streambuf& buf)
	{
		boost::asio::write(socket_,buf);
	}
	void read(ReadHandler handler)
	{
		reader_->read_starter(socket_,handler);
	}
	void async_connect(boost::asio::ip::tcp::resolver::iterator& ep_iterator,ConnectHandler handler)
	{
		boost::asio::async_connect(socket_,
			ep_iterator,
			boost::bind(handler,
				boost::asio::placeholders::error));
	}
	void async_write(boost::asio::streambuf& buf,WriteHandler handler)
	{
		boost::asio::async_write(socket_,
			buf,
			boost::bind(handler,
				boost::asio::placeholders::error));
	}
	void async_read(ReadHandler handler)
	{
		reader_->async_read_starter(socket_,handler);
	}
private:
	tcp_socket socket_;

};

} // namespace application_layer
} // namespace protocol
} // namespace oauth

#endif
