#ifndef BOOSTCONNECT_SERVER_IPP
#define BOOSTCONNECT_SERVER_IPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "../server.hpp"

namespace bstcon{

server::server(io_service &io_service,unsigned short port)
    : io_service_(&io_service),is_ctx_(false),port_(port),acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}
#ifdef USE_SSL_BOOSTCONNECT
typedef boost::asio::ssl::context context;
server::server(io_service &io_service,context &ctx,unsigned short port)
    : io_service_(&io_service),is_ctx_(true),ctx_(&ctx),port_(port),acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}
#endif

void server::start(RequestHandler handler)
{
    if(!is_ctx_)
    {
        boost::shared_ptr<SessionType> new_session(new SessionType(*io_service_));

        acceptor_.async_accept(new_session->lowest_layer(),
            boost::bind(&server::handle_accept,this,
                new_session,
                handler,
                boost::asio::placeholders::error));
    }
#ifdef USE_SSL_BOOSTCONNECT
    else
    {
        boost::shared_ptr<SessionType> new_session(new SessionType(*io_service_,*ctx_));

        acceptor_.async_accept(new_session->lowest_layer(),
            boost::bind(&server::handle_accept,this,
                new_session,
                handler,
                boost::asio::placeholders::error));
    }
#endif
    return;
}

void server::handle_accept(boost::shared_ptr<session::session_base> new_session,RequestHandler handler,const boost::system::error_code& ec)
{
    start(handler);
    manage_.run(new_session);

    if(!ec) new_session->start(handler,boost::bind(&server::handle_closed,this,_1));
    else new_session->end([](boost::shared_ptr<session::session_base>&)->void{return;});
}

void server::handle_closed(boost::shared_ptr<session::session_base>& session)
{
    manage_.stop(session);
    return;
}

} // namespace bstcon

#endif
