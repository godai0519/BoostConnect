//
// ssl_session.hpp
// ~~~~~~~~~~
//
// SSL通信で特別に行われることを行う
//

#ifndef TWIT_LIB_PROTOCOL_APPLAYER_SOCKET
#define TWIT_LIB_PROTOCOL_APPLAYER_SOCKET

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace oauth{
namespace protocol{
namespace application_layer{

class socket_base : boost::noncopyable{
public:
	typedef boost::asio::io_service io_service;
	typedef boost::system::error_code error_code;
	
	typedef boost::asio::ip::tcp::socket tcp_socket_type;
	typedef boost::asio::ssl::stream<tcp_socket_type> ssl_socket_type;
	typedef boost::asio::ssl::context context_type;
	typedef boost::asio::ip::tcp::endpoint endpoint_type;
	typedef boost::asio::ip::tcp::resolver::iterator resolve_iterator;
	typedef boost::asio::ssl::stream_base::handshake_type handshake_type;
	typedef boost::asio::socket_base::shutdown_type shutdown_type;
	typedef tcp_socket_type::lowest_layer_type lowest_layer_type;

	typedef boost::asio::mutable_buffers_1 mutable_buffer;
	typedef boost::asio::const_buffers_1   const_buffer;

	typedef boost::function<void(const error_code&)> ConnectHandler;
	typedef boost::function<void(const error_code&)> HandshakeHandler;
	typedef boost::function<void(const error_code&,std::size_t)> ReadHandler;
	typedef boost::function<void(const error_code&,std::size_t)> WriteHandler;

	socket_base(){}
	virtual ~socket_base(){}

	virtual io_service& get_io_service() = 0;
	virtual lowest_layer_type& lowest_layer() = 0;
	
	virtual error_code connect(endpoint_type&,error_code&) = 0;
	virtual void async_connect(endpoint_type&,ConnectHandler) = 0;

	virtual void handshake(handshake_type) = 0;
	virtual void async_handshake(handshake_type,HandshakeHandler) = 0;

	virtual std::size_t read_some(const mutable_buffer&,error_code&) = 0;
	virtual std::size_t write_some(const const_buffer&,error_code&) = 0;
	virtual void async_read_some(const mutable_buffer&,ReadHandler) = 0;
	virtual void async_write_some(const const_buffer&,WriteHandler) = 0;

	virtual void close() = 0;
	virtual void shutdown(shutdown_type what) = 0;
};

template<class Socket>
class socket_common : public socket_base{
public:
	socket_common(io_service& io_service) : socket_(io_service){}
	socket_common(io_service& io_service,context_type& ctx) : socket_(io_service,ctx){}
	virtual ~socket_common(){}
	
	lowest_layer_type& lowest_layer()
	{
		return socket_.lowest_layer();
	}

	io_service& get_io_service()
	{
		return socket_.get_io_service();
	}

	std::size_t read_some(const mutable_buffer& buf,error_code& ec)
	{
		return socket_.read_some(buf,ec);
	}
	std::size_t write_some(const const_buffer& buf,error_code& ec)
	{
		return socket_.write_some(buf,ec);
	}
	void async_read_some(const mutable_buffer& buf,ReadHandler handler)
	{
		socket_.async_read_some(buf,handler);
		return;
	}
	void async_write_some(const const_buffer& buf,WriteHandler handler)
	{
		socket_.async_write_some(buf,handler);
		return;
	}

protected:
	Socket socket_;
};

class tcp_socket : public socket_common<socket_base::tcp_socket_type>{
	typedef socket_common<socket_base::tcp_socket_type> my_base;
public:	
	tcp_socket(io_service& io_service) : my_base(io_service){}
	virtual ~tcp_socket(){}

	//TCP通信のコネクション確立
	error_code connect(endpoint_type& begin,error_code& ec)
	{
		return socket_.connect(begin,ec);
	}
	void async_connect(endpoint_type& begin,ConnectHandler handler)
	{
		socket_.async_connect(begin,handler);
		return;
	}

	//TCP通信ではHandshakeを行わない -> Handlerを直接呼び出す
	void handshake(handshake_type){return;}
	void async_handshake(handshake_type,HandshakeHandler handler){handler(error_code());return;}

	//TCPレイヤーの処理
	void close()
	{
		socket_.close();
		return;
	}
	void shutdown(shutdown_type what)
	{
		socket_.shutdown(what);
		return;
	}
};

class ssl_socket : public socket_common<socket_base::ssl_socket_type>{
	typedef socket_common<socket_base::ssl_socket_type> my_base;
public:
	ssl_socket(io_service& io_service,context_type& ctx) : my_base(io_service,ctx){}
	virtual ~ssl_socket(){}

	//SSL通信のコネクション確立(TCPレイヤーでコネクションを行う)
	error_code connect(endpoint_type& begin,error_code& ec)
	{
		return socket_.lowest_layer().connect(begin,ec);
	}
	void async_connect(endpoint_type& begin,ConnectHandler handler)
	{
		socket_.lowest_layer().async_connect(begin,handler);
		return;
	}
	
	void handshake(handshake_type type)
	{
		socket_.handshake(type);
		return;
	}
	void async_handshake(handshake_type type,HandshakeHandler handler)
	{
		socket_.async_handshake(type,handler);
		return;
	}
	
	//[(SSLレイヤーの処理 ->] TCPレイヤーの処理
	void close()
	{
		socket_.lowest_layer().close();
		return;
	}
	void shutdown(shutdown_type what)
	{
		socket_.shutdown();
		socket_.lowest_layer().shutdown(what);
		return;
	}
};

} // namespace application_layer
} // namespace protocol
} // namespace oauth

#endif
