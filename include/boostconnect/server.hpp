//
// server.hpp
// ~~~~~~~~~~
//
// サーバー接続のメイン管理クラス
//

#ifndef BOOSTCONNECT_SERVER
#define BOOSTCONNECT_SERVER

#include "manager.hpp"
#include "session/session_base.hpp"
#include "session/http_session.hpp"

namespace bstcon{

class server : boost::noncopyable{
private:
    typedef session::http_session SessionType;
    manager<session::session_base> manage_;

public:
    typedef boost::asio::io_service io_service;
    typedef boost::system::error_code error_code;
    typedef session::http_session::RequestHandler RequestHandler;
    typedef session::http_session::CloseHandler CloseHandler;

    server(io_service &io_service,unsigned short port)
        : io_service_(&io_service),is_ctx_(false),port_(port),acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {}
#ifdef USE_SSL_BOOSTCONNECT
    typedef boost::asio::ssl::context context;
    server(io_service &io_service,context &ctx,unsigned short port)
        : io_service_(&io_service),is_ctx_(true),ctx_(&ctx),port_(port),acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {}
#endif

    void start(RequestHandler handler)
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

private:
    void handle_accept(boost::shared_ptr<session::session_base> new_session,RequestHandler handler,const boost::system::error_code& ec)
    {
        start(handler);
        manage_.run(new_session);

        if(!ec) new_session->start(handler,boost::bind(&server::handle_closed,this,_1));
        else new_session->end([](boost::shared_ptr<session::session_base>&)->void{return;});
    }

    void handle_closed(boost::shared_ptr<session::session_base>& session)
    {
        manage_.stop(session);
        return;
    }

    io_service* io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    const unsigned short port_;
    bool is_ctx_;
#ifdef USE_SSL_BOOSTCONNECT
    context *ctx_;
#endif
};

} // namespace bstcon

#endif
