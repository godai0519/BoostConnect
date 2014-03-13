//
// client.hpp
// ~~~~~~~~~~
//
// Main Client Connection provide class
//

#ifndef BOOSTCONNECT_CLIENT_HPP
#define BOOSTCONNECT_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <boostconnect/application_layer/tcp_socket.hpp>
#include <boostconnect/application_layer/ssl_socket.hpp>
#include <boostconnect/connection_type/connection_base.hpp>

namespace bstcon{

template<typename Connection, typename Protocol>
class client : private boost::noncopyable, public boost::enable_shared_from_this<client<Connection, Protocol>>{
public:
    typedef boost::asio::io_service             io_service;
    typedef boost::system::error_code           error_code;
    typedef boost::asio::ip::tcp::endpoint      endpoint_type;
    typedef boost::shared_ptr<bstcon::response> response_type;

    typedef boost::shared_ptr<io_service>                               io_service_ptr;
    typedef boost::shared_ptr<bstcon::application_layer::socket_base>   socket_ptr;
    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;
    typedef boost::shared_ptr<Protocol>                                 protocol_ptr;

    typedef boost::function<void(protocol_ptr const&, error_code const&)> ConnectionHandler;

    client() : io_service_(boost::make_shared<io_service>()){}
    client(io_service_ptr const& io_service) : io_service_(io_service){}
    virtual ~client() = default;

    io_service_ptr get_io_service() const { return io_service_; }
        
    std::future<protocol_ptr> operator() (std::string const& host, ConnectionHandler const& handler = ConnectionHandler())
    {
        auto const socket = boost::make_shared<bstcon::application_layer::tcp_socket>(*io_service_);
        return connect_socket(socket, host, handler);
    }

#ifdef USE_SSL_BOOSTCONNECT
    typedef boost::asio::ssl::context context_type;
    std::future<protocol_ptr> operator() (std::string const& host, context_type& ctx, ConnectionHandler const& handler = ConnectionHandler())
    {
        auto const socket = boost::make_shared<bstcon::application_layer::ssl_socket>(*io_service_, ctx);
        return connect_socket(socket, host, handler);
    }
#endif

private:
    std::future<protocol_ptr> connect_socket(socket_ptr const& socket, std::string const& host, ConnectionHandler const& handler)
    {
        auto self = this->shared_from_this();
        auto const promise = boost::make_shared<std::promise<protocol_ptr>>();
        auto const connection = boost::make_shared<Connection>(socket);
        connection->connect(
            host, 
            [self, promise, handler](connection_ptr const& connection, error_code const& ec)
            {
                auto const protocol = boost::make_shared<Protocol>(connection);

                if(handler) handler(protocol, ec);
                promise->set_value(protocol);

                return;
            }
        );
        return promise->get_future();
    }

    io_service_ptr const io_service_;
};

} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/client.ipp"
#endif

#endif
