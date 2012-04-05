//
// server.hpp
// ~~~~~~~~~~
//
// ê⁄ë±ÇÃÉÅÉCÉìä«óùÉNÉâÉX
//

#ifndef TWIT_LIB_PROTOCOL_SERVER
#define TWIT_LIB_PROTOCOL_SERVER

/*#include "application_layer/session_base.hpp"
#include "application_layer/tcp_session.hpp"
#include "application_layer/ssl_session.hpp"*/

#include "session/manager.hpp"
#include "session/http_session.hpp"

namespace oauth{
namespace protocol{

class server : boost::noncopyable{
private:
	typedef session::http_session SessionType;
	session::manager<typename SessionType> manage_;

public:
	typedef boost::asio::io_service io_service;
	typedef boost::system::error_code error_code;
#ifndef NO_SSL
	typedef boost::asio::ssl::context context;
#endif
	
	typedef session::http_session::RequestHandler RequestHandler;
	typedef session::http_session::CloseHandler CloseHandler;

	server(io_service &io_service,unsigned short port)
		: io_service_(&io_service),ctx_(nullptr),port_(port),acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	{}
#ifndef NO_SSL
	server(io_service &io_service,context &ctx,unsigned short port)
		: io_service_(&io_service),ctx_(&ctx),port_(port),acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	{}
#endif

	void start(RequestHandler handler)
	{
		//application_layer::session_base* new_session = new application_layer::session_base(io_service_, context_);
		//application_layer::session_base* new_session;
		if(ctx_ == nullptr)
		{
			boost::shared_ptr<SessionType> new_session(new SessionType(*io_service_));
			//auto socket = new_session->socket();

			acceptor_.async_accept(new_session->lowest_layer(),
				boost::bind(&server::handle_accept,this,
					new_session,
					handler,
					boost::asio::placeholders::error));
		}
		else
		{
			boost::shared_ptr<SessionType> new_session(new SessionType(*io_service_,*ctx_));
			//auto& socket = (new_session->socket()->lowest_layer());
			acceptor_.async_accept(new_session->lowest_layer(),
				boost::bind(&server::handle_accept,this,
					new_session,
					handler,
					boost::asio::placeholders::error));
		}


	}

private:
	void handle_accept(boost::shared_ptr<SessionType> new_session,RequestHandler handler,const boost::system::error_code& ec)
	{
		start(handler);
		manage_.run(new_session);

		if(!ec) new_session->start(handler,boost::bind(&server::handle_closed,this,_1));
		else new_session->end();
	}

	void handle_closed(boost::shared_ptr<SessionType>& session)
	{
		manage_.stop(session);
		return;
	}

	io_service* io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	const unsigned short port_;
#ifndef NO_SSL
	context *ctx_;
#endif
};

}
}

#endif
