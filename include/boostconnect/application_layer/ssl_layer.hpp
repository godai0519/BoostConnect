#ifndef NO_SSL
#ifndef BOOSTCONNECT_SSL_LAYER
#define BOOSTCONNECT_SSL_LAYER

#include "layer_base.hpp"

class ssl_layer : public layer_base{
public:
	ssl_layer(io_service &io_service,context &ctx) : socket_(io_service,ctx){}
	virtual ~ssl_layer(){}	

	error_code& async_connect(){error_code ec; return ec;} //—\’è‚È‚µ
	void async_write(){}
	void async_write_some(){}
	void async_read(){}
	void async_read_some(){}
	
	error_code& sync_connect(error_code& ec,endpoint& ep)
	{
		socket_.lowest_layer().connect(ep,ec);
		socket_.handshake(ssl_socket::client,ec);
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
	template<class Delim>
	error_code& sync_read_some(boost::asio::streambuf &buf,const Delim &delim,error_code& ec)
	{
		boost::asio::read_until(socket_,buf,delim,ec);
		return ec;
	}

	error_code& close(error_code& ec)
	{
		return socket_.lowest_layer().close(ec);
	}

private:
	ssl_socket socket_;
};

#endif
#endif