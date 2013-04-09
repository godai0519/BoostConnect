//
// http_session.ipp
// ~~~~~~~~~~
//
// Management a HTTP Connection's Session
//

#ifndef BOOSTCONNECT_SESSION_HTTP_IPP
#define BOOSTCONNECT_SESSION_HTTP_IPP

#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include "../http_session.hpp"
#include "../../application_layer/tcp_socket.hpp"
#include "../../application_layer/ssl_socket.hpp"

namespace bstcon{
namespace session{
    
bstcon::application_layer::socket_base::lowest_layer_type& http_session::lowest_layer()
{
    return socket_->lowest_layer();
}
    
http_session::http_session(io_service& io_service, const int deadline_second)
    : read_timer_(io_service), socket_(new bstcon::application_layer::tcp_socket(io_service)), deadline_second_(deadline_second)
{
}
#ifdef USE_SSL_BOOSTCONNECT
typedef boost::asio::ssl::context context;
http_session::http_session(io_service& io_service, context& ctx, const int deadline_second)
    : read_timer_(io_service), socket_(new bstcon::application_layer::ssl_socket(io_service,ctx)), deadline_second_(deadline_second)
{
}
#endif

http_session::~http_session(){} // = default;

void http_session::start(RequestHandler handler,CloseHandler c_handler)
{
    // mutex利用
    //if(socket_busy_) return; //例外予定
    //socket_busy_ = true;
    handler_ = handler;
    c_handler_ = c_handler;
        
#ifdef USE_SSL_BOOSTCONNECT
    socket_->async_handshake(boost::asio::ssl::stream_base::server,
        boost::bind(&http_session::handle_handshake,shared_from_this(),
            boost::asio::placeholders::error));
#else
    socket_->async_handshake(
        boost::bind(&http_session::handle_handshake,shared_from_this(),
            boost::asio::placeholders::error));
#endif
}
void http_session::end(CloseHandler c_handler)
{
    socket_->close();

    c_handler(static_cast<boost::shared_ptr<session_base>>(shared_from_this()));
    return;
}

std::future<void> http_session::set_headers(int status_code, const std::string& status_message, const std::string& http_version, const std::map<std::string,std::string>& header)
{
    namespace karma = boost::spirit::karma;

    auto write_buf = boost::make_shared<boost::asio::streambuf>();
    std::ostreambuf_iterator<char> os(write_buf.get());
    bool r = karma::generate(os,
        "HTTP/" << karma::string << ' ' << karma::int_ << ' ' << karma::string << "\r\n",
        http_version,
        status_code,
        status_message);

    if(!header.empty())
    {
        r = r && karma::generate(os,
            +(karma::string << ": " << karma::string << "\r\n") << "\r\n",
            header);
    }

    auto p = boost::make_shared<std::promise<void>>();
    boost::asio::async_write(*socket_, *write_buf,
        boost::bind(&http_session::handle_write, shared_from_this(),
            p, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, write_buf));

    return p->get_future();
}

std::future<void> http_session::set_body(const std::string& body)
{
    auto write_buf = boost::make_shared<boost::asio::streambuf>();
    std::ostream os(write_buf.get());
    os << body;

    auto p = boost::make_shared<std::promise<void>>();
    boost::asio::async_write(*socket_, *write_buf,
        boost::bind(&http_session::handle_end, shared_from_this(),
            p, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, write_buf));

    return p->get_future();
}

std::future<void> http_session::set_chunk(const std::string& size, const std::string& body)
{
    auto write_buf = boost::make_shared<boost::asio::streambuf>();
    std::ostream os(write_buf.get());
    os << size << "\r\n";

    auto p = boost::make_shared<std::promise<void>>();
    if(size == "0")
    {
        boost::asio::async_write(*socket_, *write_buf,
            boost::bind(&http_session::handle_end, shared_from_this(),
                p, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, write_buf));
    }
    else
    {
        os << body << "\r\n";
        boost::asio::async_write(*socket_, *write_buf,
            boost::bind(&http_session::handle_write, shared_from_this(),
                p, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, write_buf));
    }

    return p->get_future();
}

const int http_session::int_parser(const std::string& base) const
{
    namespace qi = boost::spirit::qi;

    int parsed = 0;
    qi::parse(base.cbegin(),base.cend(),qi::int_,parsed);

    return parsed;
}

const int http_session::request_header_parser(const std::string& request_str, const boost::shared_ptr<bstcon::request>& request_containar) const
{
    namespace qi = boost::spirit::qi;

    std::string::const_iterator it = request_str.cbegin();

    qi::parse(
        it,
        request_str.cend(),
        +(qi::char_-' ') >> ' ' >> +(qi::char_-' ') >> ' ' >> +(qi::char_-"\r\n") >> "\r\n",
        request_containar->method, request_containar->path, request_containar->http_version
        );

    qi::parse(
        it,
        request_str.cend(),
        *( +(qi::char_-':') >> ": " >> +(qi::char_-"\r\n") >> "\r\n" ) >> "\r\n",
        request_containar->header
        );

    return std::distance(request_str.cbegin(),it);
}

void http_session::handle_handshake(const error_code& ec)
{
    if(!ec)
    {
        auto read_buf = boost::make_shared<boost::asio::streambuf>();

        boost::asio::async_read_until(*socket_, *read_buf,
            "\r\n\r\n",
            boost::bind(&http_session::handle_header_read,shared_from_this(),
                read_buf,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

        read_timer_.expires_from_now(boost::posix_time::seconds(deadline_second_));
        read_timer_.async_wait(
            boost::bind(&http_session::read_timeout, shared_from_this(), boost::asio::placeholders::error));

    }
    //else delete this; //例外
}

void http_session::handle_header_read(const boost::shared_ptr<boost::asio::streambuf> read_buf, const error_code& ec, const std::size_t sz)
{
    if(!ec)
    {
        auto request = boost::make_shared<bstcon::request>();
        std::string request_str(boost::asio::buffer_cast<const char*>(read_buf->data()));

        //リクエストヘッダーのパース
        const int header_size = request_header_parser(request_str, request);
        request_str.erase(0,header_size);
        request->body.append(request_str);

        read_buf->consume(header_size);
        
        size_t byte = int_parser(find_return_or_default(request->header,"Content-Length","0"));
        if( byte != 0 )
        {
            //長さがある
            boost::asio::async_read(
                *socket_,
                *read_buf,
                boost::asio::transfer_at_least(byte - request->body.length()),
                boost::bind(&http_session::handle_body_read,shared_from_this(),
                    read_buf,
                    request,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            //長さがない or 0 -> 読まずにhandle_body_readへ
            handle_body_read(read_buf, request, ec, sz);
        }
    }
    //else delete this; //例外
}

void http_session::handle_body_read(const boost::shared_ptr<boost::asio::streambuf> read_buf, const boost::shared_ptr<bstcon::request> request, const error_code& ec, const std::size_t sz)
{
    if(!ec)
    {
        std::string body_str(boost::asio::buffer_cast<const char*>(read_buf->data()));
        request->body.append(body_str);

        handle_request_read_complete(request);
    }
    //else delete this; //例外
}

void http_session::handle_request_read_complete(const boost::shared_ptr<bstcon::request> request)
{
    keep_alive_ = 
        equal_without_capital(find_return_or_default(request->header,"Connection","close"),      "Keep-Alive") ||
        equal_without_capital(find_return_or_default(request->header,"Proxy-Connection","close"),"Keep-Alive");
    
    handler_(request, shared_from_this());

    return;
}

void http_session::handle_write(const boost::shared_ptr<std::promise<void>> p, const error_code& ec, const std::size_t sz, const boost::shared_ptr<boost::asio::streambuf> buf)
{
    p->set_value();
    return;
}
void http_session::handle_end(const boost::shared_ptr<std::promise<void>> p, const error_code& ec, const std::size_t sz, const boost::shared_ptr<boost::asio::streambuf> buf)
{
    if(!ec)
    {
        if(keep_alive_)
        {
            p->set_value();
            handle_handshake(error_code());
        }
        else
        {
            this->end(c_handler_);
            p->set_value();
            return;
        }
    }

    return;
}

void http_session::read_timeout(const error_code& ec)
{
    this->end(c_handler_);
    return;
}

} // namespace session
} // namespace bstcon

#endif
