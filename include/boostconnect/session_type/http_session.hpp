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
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include "session_base.hpp"
#include <boostconnect/connection_type/connection_base.hpp>

namespace bstcon{
namespace session{
    
class http_session : public session_common<http_session> {
public:
    typedef boost::system::error_code error_code;

    typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
    typedef boost::shared_ptr<http_session> session_ptr;
    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

    typedef boost::function<void(boost::shared_ptr<bstcon::request> const&, session_ptr const&)> RequestHandler;
    typedef boost::function<void(session_ptr const&)>                                            WriteHandler;
    typedef boost::function<void(session_ptr const&)>                                            CloseHandler;

    http_session(io_service_ptr const& io_service, connection_ptr const& connection, unsigned int const timeout_second, RequestHandler const request_handler, CloseHandler const close_handler);
    virtual ~http_session() = default;

    void read();
    void close();
    
    std::future<void> set_all(boost::shared_ptr<bstcon::response> const& header, WriteHandler const handler = WriteHandler());
    std::future<void> set_headers(boost::shared_ptr<bstcon::response> const& header, WriteHandler const handler = WriteHandler());
    std::future<void> set_body(std::string const& body, WriteHandler const handler = WriteHandler());

    std::future<void> set_chunk(WriteHandler const handler = WriteHandler());
    std::future<void> set_chunk(std::string const& chunk, WriteHandler const handler = WriteHandler());
    
private:
    std::future<void> write_buffer(boost::shared_ptr<boost::asio::streambuf> const& buffer, WriteHandler const handler);

    //boost::asio::deadline_timer timer_;

    unsigned int   const timeout_second_;
    io_service_ptr const io_service_;
    connection_ptr const connection_;
    RequestHandler const request_handler_;
    CloseHandler   const close_handler_;
};

} // namespace session
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/http_session.ipp"
#endif

#endif
