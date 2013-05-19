//
// connection_base.hpp
// ~~~~~~~~~~
//
// Provide Sync/ASync Connection Common Class powered by Boost.Asio
//

#ifndef BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE_HPP
#define BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE_HPP

#include <future>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional/optional.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../application_layer/socket_base.hpp"
#include "../request.hpp"
#include "../response.hpp"

namespace bstcon{
namespace connection_type{

enum connection_type{ async, sync };

class reader : boost::noncopyable
{
public:
    typedef boost::system::error_code                        error_code;
    typedef boost::shared_ptr<bstcon::response>              response_type;
    typedef boost::function<bool(response_type, error_code)> ChunkHandler;
    typedef boost::function<void(error_code)>                EndHandler;

	reader();
	virtual ~reader();
	
	//Util
	const response_type& get_response() const;
	const response_type& reset_response();
	bool is_chunked() const;

	//
	// 共通メンバ関数
	//
protected:
	const int read_header(const std::string& source);
	const int chunk_parser(const std::string& source, std::size_t& chunk);

	//
	// 同期
	//
public:
	template<class Socket>
	void read_starter(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler);
	
protected:
	template<class Socket>
	void read_chunk_size(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler);
	
	template<class Socket>
	void read_chunk_body(Socket& socket, const std::size_t chunk, EndHandler end_handler, ChunkHandler chunk_handler);

	//
	// 非同期
	//
public:
	template<class Socket>
	void async_read_starter(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler);

protected:
	template<class Socket>
	void async_read_header(Socket& socket, const error_code& ec, const std::size_t, EndHandler handler, ChunkHandler chunk_handler);

	template<class Socket>
	void async_read_all(Socket& socket, const error_code& ec, const std::size_t size, EndHandler handler);
	
	template<class Socket>
	void async_read_chunk_size(Socket& socket, const error_code& ec, const std::size_t, EndHandler handler, ChunkHandler chunk_handler);

	template<class Socket>
	void async_read_chunk_body(Socket& socket, const std::size_t chunk, const error_code& ec, const std::size_t, EndHandler handler, ChunkHandler chunk_handler);

	template<class Socket>
	void async_read_end(Socket& socket, const error_code &ec, const std::size_t, EndHandler handler);
	
private:
    boost::optional<std::string> get_headers_value(const std::string& key) const;

	boost::asio::streambuf read_buf_;
	response_type response_;
};

class connection_base : boost::noncopyable{
public:
    typedef boost::system::error_code           error_code;
    typedef boost::asio::ip::tcp::endpoint      endpoint_type;
    typedef boost::shared_ptr<bstcon::response> response_type;
    typedef bstcon::request                     request_type;

    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

    typedef boost::function<void(connection_ptr, error_code)> ConnectionHandler;
    typedef boost::function<bool(response_type, error_code)>  ChunkHandler;
    typedef boost::function<void(response_type, error_code)>  EndHandler;
    
    connection_base();
    virtual ~connection_base();

    //通信開始(オーバーライド必須)
    virtual connection_ptr connect(const std::string&, ConnectionHandler) = 0;
    virtual connection_ptr connect(const endpoint_type&, ConnectionHandler) = 0;
    
    virtual std::future<response_type> send(
        boost::shared_ptr<boost::asio::streambuf>,
        EndHandler = [](response_type, error_code)->void{},
        ChunkHandler = [](response_type, error_code)->bool{ return true; }
        ) = 0;

    std::future<response_type> send(
        const request_type& request,
        EndHandler end_handler = [](response_type, error_code)->void{},
        ChunkHandler chunk_handler = [](response_type, error_code)->bool{ return true; }
        );

    virtual void close();

protected:
    std::string host_;
    boost::shared_ptr<application_layer::socket_base> socket_;
    std::unique_ptr<reader> reader_;
};

template <class Devide>
class connection_common : public connection_base, public boost::enable_shared_from_this<Devide>{};

} // namespace connection_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/connection_base.ipp"
#endif

#endif
