//
// server.hpp
// ~~~~~~~~~~
//
// Main Sever Connection provide class
//

#ifndef BOOSTCONNECT_SERVER_HPP
#define BOOSTCONNECT_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boostconnect/request.hpp>
#include <boostconnect/application_layer/tcp_socket.hpp>
#include <boostconnect/application_layer/ssl_socket.hpp>
#include <boostconnect/connection_type/async_connection.hpp>

namespace bstcon{

template<class Session>
class server : boost::noncopyable
{
public:
    typedef boost::asio::io_service       io_service;
    typedef boost::shared_ptr<io_service> io_service_ptr;
    typedef boost::asio::ip::tcp::endpoint endpoint_type;
    typedef boost::asio::ip::tcp::acceptor acceptor;
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<bstcon::application_layer::socket_base> socket_ptr;
    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

    typedef boost::shared_ptr<Session> session_ptr;
    
    typedef boost::function<void(boost::shared_ptr<bstcon::request> const&, session_ptr const&)> RequestHandler;

#ifdef USE_SSL_BOOSTCONNECT
    typedef boost::asio::ssl::context context_type;
    typedef boost::shared_ptr<context_type> context_ptr;

    server(unsigned short port, unsigned int timeout_second = 30) : server(boost::make_shared<io_service>(), port, timeout_second){}
    server(io_service_ptr const& io_service, unsigned short port, unsigned int timeout_second = 30)
        : io_service_(io_service), ctx_(), port_(port), timeout_second_(timeout_second), is_ssl_(false){}

    server(context_ptr const& ctx, unsigned short port, unsigned int timeout_second = 30) : server(ctx, boost::make_shared<io_service>(), port, timeout_second){}
    server(io_service_ptr const& io_service, context_ptr const& ctx, unsigned short port, unsigned int timeout_second = 30)
        : io_service_(io_service), ctx_(ctx), port_(port), timeout_second_(timeout_second), is_ssl_(true){}
#else
    server(unsigned short port, unsigned int timeout_second = 30) : server(boost::make_shared<io_service>(), port, timeout_second){}
    server(io_service_ptr const& io_service, unsigned short port, unsigned int timeout_second = 30)
        : io_service_(io_service), port_(port), timeout_second_(timeout_second){}
#endif

    virtual ~server() = default;
    
    void set_timeout(unsigned int second){ if(second > 0) this->timeout_second_ = second; }
    unsigned int timeout(){ return this->timeout_second_; }

    io_service_ptr service() const { return io_service_; }

    void start(RequestHandler handler)
    {
        acceptor_.reset(new acceptor(*io_service_, endpoint_type(boost::asio::ip::tcp::v4(), *port_))); // TODO

#ifdef USE_SSL_BOOSTCONNECT
        socket_ptr const socket = (is_ssl_)
            ? static_cast<socket_ptr>(boost::make_shared<bstcon::application_layer::ssl_socket>(*io_service_, *ctx_))
            : static_cast<socket_ptr>(boost::make_shared<bstcon::application_layer::tcp_socket>(*io_service_));
#else
        socket_ptr const socket = boost::make_shared<bstcon::application_layer::tcp_socket>(*io_service_);
#endif

        acceptor_->async_accept(
            socket->lowest_layer(),
            [this, socket, handler](error_code const& ec)
            {
                if(ec)
                {
                    socket->close();
                }
                else
                {
                    boost::make_shared<bstcon::connection_type::async_connection>(socket)->accepted(
                        [this, handler](connection_ptr connection, error_code ec)
                        {
                            auto const session = boost::make_shared<Session>(
                                io_service_, connection, timeout_second_, handler,
                                [](session_ptr const&)
                                {
                                }
                            );
                            session->read();

                            // コントローラーにでも突っ込むかぁ？
                        }
                    );
                }

                return;
            }
        );

        return;
    }

#ifdef USE_SSL_BOOSTCONNECT
    boost::shared_ptr<boost::asio::ssl::context> const ctx_;
    bool const is_ssl_;
#endif

    boost::scoped_ptr<acceptor> acceptor_;
    io_service_ptr const io_service_;
    
    boost::optional<unsigned int> const port_;

    unsigned int timeout_second_;
};

} // namespace bstcon

#endif
