//
// http_session.hpp
// ~~~~~~~~~~
//
// HTTP通信のセッションを管理
//

#ifndef BOOSTCONNECT_SESSION_HTTP_HPP
#define BOOSTCONNECT_SESSION_HTTP_HPP

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include "session_base.hpp"
#include "../application_layer/socket_base.hpp"

namespace bstcon{
namespace session{
    
class http_session : public session_common<http_session> {
public:
    typedef boost::asio::io_service io_service;
    typedef boost::system::error_code error_code;

    bstcon::application_layer::socket_base::lowest_layer_type& lowest_layer();    
    http_session(io_service& io_service);

#ifdef USE_SSL_BOOSTCONNECT
    typedef boost::asio::ssl::context context;
    http_session(io_service& io_service,context& ctx);
#endif
    virtual ~http_session();

    void start(RequestHandler handler,CloseHandler c_handler);
    void end(CloseHandler c_handler);

private:
    void handle_handshake(const error_code& ec);

    void read_timeout(const error_code& ec);

    void handle_header_read(const error_code& ec,std::size_t);
    void handle_body_read(const error_code& ec,std::size_t sz);
    void handle_request_read_complete();
    void handle_write(const error_code& ec,std::size_t sz);

    template<class OutputIterator>
    bool response_generater(OutputIterator& sink,const response_type& response) const;
    const int int_parser(const std::string& base) const;

    const int request_header_parser(const std::string& request_str,bstcon::request& request_containar) const;

    request_type request_;

    std::unique_ptr<boost::asio::streambuf> read_buf_;
    std::unique_ptr<boost::asio::streambuf> write_buf_;

    bstcon::application_layer::socket_base *socket_;
    RequestHandler handler_;
    CloseHandler c_handler_;
    boost::asio::deadline_timer read_timer_;
    bool socket_busy_;
    bool keep_alive_;
};

} // namespace session
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include <boostconnect/session/impl/http_session.ipp>
#endif

#endif
