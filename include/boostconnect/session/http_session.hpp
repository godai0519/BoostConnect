//
// http_session.hpp
// ~~~~~~~~~~
//
// Management a HTTP Connection's Session
//

#ifndef BOOSTCONNECT_SESSION_HTTP_HPP
#define BOOSTCONNECT_SESSION_HTTP_HPP

#include <future>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include "session_base.hpp"
#include "../application_layer/socket_base.hpp"

namespace bstcon{
namespace session{
    
class http_session : public session_common<http_session> {
public:
    typedef boost::asio::io_service io_service;
    typedef boost::system::error_code error_code;

    bstcon::application_layer::socket_base::lowest_layer_type& lowest_layer();    
    http_session(io_service& io_service, const int deadline_second = 5);

#ifdef USE_SSL_BOOSTCONNECT
    typedef boost::asio::ssl::context context;
    http_session(io_service& io_service, context& ctx, const int deadline_second = 5);
#endif
    virtual ~http_session();

    void start(RequestHandler handler,CloseHandler c_handler);
    void end(CloseHandler c_handler);

    std::future<void> set_headers(int status_code, const std::string& status_message, const std::string& http_version="1.0", const std::map<std::string,std::string>& header = std::map<std::string,std::string>());
    std::future<void> set_body(const std::string& body);
    std::future<void> set_chunk(const std::string& size, const std::string& body);
    
    const int int_parser(const std::string& base) const;
    const int request_header_parser(const std::string& request_str, const boost::shared_ptr<bstcon::request>& request_containar) const;

private:
    void handle_handshake(const error_code& ec);
    void handle_header_read(const boost::shared_ptr<boost::asio::streambuf> read_buf, const error_code& ec, const std::size_t sz);
    void handle_body_read(const boost::shared_ptr<boost::asio::streambuf> read_buf, const boost::shared_ptr<bstcon::request> request, const error_code& ec, const std::size_t sz);
    void handle_request_read_complete(const boost::shared_ptr<bstcon::request> request);

    void handle_write(const boost::shared_ptr<std::promise<void>> p, const error_code& ec, const std::size_t sz, const boost::shared_ptr<boost::asio::streambuf> buf);
    void handle_end(const boost::shared_ptr<std::promise<void>> p, const error_code& ec, const std::size_t sz, const boost::shared_ptr<boost::asio::streambuf> buf);

    void read_timeout(const error_code& ec);

    boost::scoped_ptr<bstcon::application_layer::socket_base> socket_;
    boost::asio::deadline_timer read_timer_;
    const int deadline_second_;

    RequestHandler handler_;
    CloseHandler c_handler_;
    bool keep_alive_;
};

} // namespace session
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/http_session.ipp"
#endif

#endif
