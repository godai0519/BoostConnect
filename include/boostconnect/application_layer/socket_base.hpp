//
// socket_base.hpp
// ~~~~~~~~~~
//
// TCP/SSL/TLS sockets wrapper(common) classes
//

#ifndef BOOSTCONNECT_APPLAYER_SOCKET_BASE_HPP
#define BOOSTCONNECT_APPLAYER_SOCKET_BASE_HPP

#include <boost/asio.hpp>
#include <boost/function.hpp>

#ifdef USE_SSL_BOOSTCONNECT
#include <boost/asio/ssl.hpp>
#endif

namespace bstcon{
namespace application_layer{

struct socket_base : boost::noncopyable{
    typedef boost::asio::io_service io_service;
    typedef boost::system::error_code error_code;
    
    typedef boost::asio::ip::tcp::socket tcp_socket_type;
#ifdef USE_SSL_BOOSTCONNECT
    typedef boost::asio::ssl::stream<tcp_socket_type> ssl_socket_type;
    typedef boost::asio::ssl::context context_type;
    typedef boost::asio::ssl::stream_base::handshake_type handshake_type;
#endif
    typedef boost::asio::ip::tcp::endpoint endpoint_type;
    typedef boost::asio::ip::tcp::resolver::iterator resolve_iterator;
    typedef boost::asio::socket_base::shutdown_type shutdown_type;
    typedef tcp_socket_type::lowest_layer_type lowest_layer_type;

    typedef boost::asio::mutable_buffers_1 mutable_buffer;
    typedef boost::asio::const_buffers_1     const_buffer;
    typedef boost::asio::detail::consuming_buffers<boost::asio::const_buffer,boost::asio::basic_streambuf<>::const_buffers_type> consuming_buffer;

    typedef boost::function<void(const error_code&)> ConnectHandler;
    typedef boost::function<void(const error_code&)> HandshakeHandler;
    typedef boost::function<void(const error_code&,std::size_t)> ReadHandler;
    typedef boost::function<void(const error_code&,std::size_t)> WriteHandler;

    socket_base();
    virtual ~socket_base();

    virtual io_service& get_io_service() = 0;
    virtual lowest_layer_type& lowest_layer() = 0;
    virtual const std::string service_protocol() const = 0;

    virtual error_code& connect(endpoint_type&,error_code&) = 0;
    virtual void async_connect(endpoint_type&,ConnectHandler) = 0;
    
#ifdef USE_SSL_BOOSTCONNECT
    virtual void handshake(handshake_type) = 0;
    virtual void async_handshake(handshake_type,HandshakeHandler) = 0;
#else
    virtual void handshake() = 0;
    virtual void async_handshake(HandshakeHandler) = 0;
#endif

    virtual std::size_t read_some(const mutable_buffer&,error_code&) = 0;
    virtual std::size_t write_some(const const_buffer&,error_code&) = 0;
    virtual std::size_t write_some(const consuming_buffer&,error_code&) = 0;

    virtual void async_read_some(const mutable_buffer&,ReadHandler) = 0;
    virtual void async_write_some(const const_buffer&,WriteHandler) = 0;
    virtual void async_write_some(const consuming_buffer&,WriteHandler) = 0;

    virtual void close() = 0;
    virtual void shutdown(shutdown_type what) = 0;
};

// TCP・SSLに共通の設定
template<class Socket>
class socket_common : public socket_base{
public:
    socket_common(io_service& io_service);
    
#ifdef USE_SSL_BOOSTCONNECT
    socket_common(io_service& io_service,context_type& ctx);
#endif

    virtual ~socket_common();
    
    lowest_layer_type& lowest_layer();
    io_service& get_io_service();

    std::size_t read_some(const mutable_buffer& buf,error_code& ec);
    std::size_t write_some(const const_buffer& buf,error_code& ec);
    std::size_t write_some(const consuming_buffer& buf,error_code& ec);
    void async_read_some(const mutable_buffer& buf,ReadHandler handler);
    void async_write_some(const const_buffer& buf,WriteHandler handler);
    void async_write_some(const consuming_buffer& buf,WriteHandler handler);

protected:
    Socket socket_;
};

} // namespace application_layer
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/socket_base.ipp"
#endif

#endif
