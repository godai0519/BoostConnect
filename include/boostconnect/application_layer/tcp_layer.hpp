#ifndef BOOSTCONNECT_TCP_LAYER
#define BOOSTCONNECT_TCP_LAYER

#include "layer_base.hpp"

//ここで基底クラスの実装を行うが。
class tcp_layer : public layer_base{
public:
	tcp_layer(io_service &io_service) : socket_(io_service){}
	virtual ~tcp_layer(){}
	
	error_code& async_connect(){error_code ec; return ec;} //予定なし
	//error_code& async_connect(error_code& ec,ConnectHandler& handler)
	//{
	//}
	void async_write(){}
	void async_write_some(){}
	void async_read(){}
	void async_read_some(){}

	error_code& sync_connect(error_code& ec,endpoint& ep)
	{
		socket_.connect(ep,ec);
		return ec;
	}
	error_code& sync_write(boost::asio::streambuf &buf,error_code& ec)
	{
		boost::asio::write(socket_,buf,ec);
		return ec;
	}
	error_code& sync_read(boost::asio::streambuf &buf,error_code& ec)
	{
		while(boost::asio::read(socket_,buf,boost::asio::transfer_at_least(1),ec));
		return ec;
	}

	error_code& close(error_code& ec)
	{
		return socket_.close(ec);
	}

private:
	tcp_socket socket_;

};

#endif
