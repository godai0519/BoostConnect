#ifndef BOOSTCONNECT_CLIENT
#define BOOSTCONNECT_CLIENT

#include <memory>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include "application_layer/layer_base.hpp"
#include "application_layer/connector.hpp"

//•¡”‚Ì’ÊM‚ğ“¯‚É—v‹‚µ‚½Û‚Ì•ÛØ‚Í‚µ‚È‚¢
class client{
public:
	typedef layer_base::io_service io_service;
	typedef layer_base::error_code error_code;
	typedef layer_base::endpoint endpoint;
#ifdef BOOSTCONNECT_SSL_LAYER
	typedef boost::asio::ssl::context context;
#endif
	//// TODO: C++11‚É‚Ä‰Â•Ï’·ˆø”‚É‘Î‰‚³‚¹‚é
	//template<class ...Args>
	//client(boost::asio::io_service &io_service,boost::asio::ip::tcp::endpoint& ep,Args... args)
	//{
	//	connector_.reset(new connector(io_service,ep,arg...));
	//	socket_layer_ = connector_->get_layer();
	//}

	//TCP
	client(io_service &io_service,endpoint& ep)
	{
		connector_.reset(new connector(io_service,ep,ec));
		socket_layer_ = connector_->get_layer();
	}

	//SSL
	client(io_service &io_service,context &ctx,boost::asio::ip::tcp::endpoint& ep)
	{
		connector_.reset(new connector(io_service,ctx,ep,ec));
		socket_layer_ = connector_->get_layer();
	}
	error_code& sync_write(boost::asio::streambuf &buf){return socket_layer_->sync_write(buf,ec);}
	error_code& sync_read(boost::asio::streambuf &buf){return socket_layer_->sync_read(buf,ec);}
	error_code& close(){return socket_layer_->close(ec);}
private:
	boost::scoped_ptr<connector> connector_;
	std::shared_ptr<layer_base> socket_layer_;
	error_code ec;
};

#endif
