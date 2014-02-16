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

#include "application_layer/socket_base.hpp"
#include "connection_type/connection_base.hpp"

namespace bstcon{

template<typename Socket, typename Connection, typename Protocol>
class client : boost::noncopyable, public boost::enable_shared_from_this<client<Socket, Connection, Protocol>>{
public:
    typedef boost::asio::io_service             io_service;
    typedef boost::system::error_code           error_code;
    typedef boost::asio::ip::tcp::endpoint      endpoint_type;
    typedef boost::shared_ptr<bstcon::response> response_type;

    typedef boost::shared_ptr<bstcon::application_layer::socket_base>   socket_ptr;
	typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;
	typedef boost::shared_ptr<Protocol>                                 protocol_ptr;

	typedef boost::function<void(protocol_ptr const&, error_code const&)> ConnectionHandler;
	//typedef bstcon::connection_type::connection_base::ConnectionHandler ConnectionHandler;

#ifdef USE_SSL_BOOSTCONNECT
	typedef boost::asio::ssl::context context;
	client(io_service &io_service, context &ctx)
		: io_service_(io_service), ctx_(&ctx)
	{
	}

	client(io_service &io_service)
		: io_service_(io_service), ctx_(boost::none)
	{
	}
#else
	client(io_service &io_service)
		: io_service_(io_service)
	{
	}
#endif

	virtual ~client() = default;
        
    //template<typename T>
	//std::future<protocol_ptr> operator() (T const& host, ConnectionHandler const& handler)
	std::future<protocol_ptr> operator() (std::string const& host, ConnectionHandler const& handler = ConnectionHandler())
	{
		const auto p = boost::make_shared<std::promise<protocol_ptr>>();
		auto self = shared_from_this();
		create_connection()->connect(
			host, 
			[self, p, handler](connection_ptr const& connection, error_code const& ec)
			{
                auto protocol = boost::make_shared<Protocol>(connection);
                if(handler) handler(protocol, ec);
				p->set_value(std::move(protocol));
				return;
			});

		return p->get_future();
	}

private:
    inline socket_ptr create_socket() const
	{
#ifdef USE_SSL_BOOSTCONNECT
		if(ctx_)
			return boost::make_shared<Socket>(io_service_, **ctx_);
		else
#endif
			return boost::make_shared<Socket>(io_service_);
	}
	
    inline connection_ptr create_connection() const
	{
		return boost::make_shared<Connection>(create_socket());
	}

#ifdef USE_SSL_BOOSTCONNECT
    boost::optional<context*> ctx_;
#endif
    boost::asio::io_service& io_service_;
};

} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/client.ipp"
#endif

#endif
