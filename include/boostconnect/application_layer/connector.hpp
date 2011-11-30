#ifndef BOOSTCONNECT_CONNECTOR
#define BOOSTCONNECT_CONNECTOR

#include "layer_base.hpp"

//通信開始、終了専用クラス(Sync)
class connector{
public:
	typedef layer_base::io_service io_service;
	typedef layer_base::error_code error_code;
	typedef layer_base::endpoint endpoint;
#ifndef NO_SSL
#endif

#ifdef BOOSTCONNECT_TCP_LAYER
	connector(io_service &io_service,endpoint& ep,error_code& ec) : socket_layer_(new tcp_layer(io_service))
	{
		socket_layer_->sync_connect(ec,ep);
	}
#endif
#ifdef BOOSTCONNECT_SSL_LAYER
	typedef boost::asio::ssl::context context;
	connector(io_service &io_service,context &ctx,endpoint& ep,error_code& ec) : ctx_(&ctx),socket_layer_(new ssl_layer(io_service,ctx))
	{
		socket_layer_->sync_connect(ec,ep);
	}
#endif
	virtual ~connector()
	{
		socket_layer_.reset();
	}
	
	error_code& close(error_code& ec){return socket_layer_->close(ec);}

	std::shared_ptr<layer_base>& get_layer()
	{
		return socket_layer_;
	}

private:
	std::shared_ptr<layer_base> socket_layer_;
	context *ctx_;
};

#endif
