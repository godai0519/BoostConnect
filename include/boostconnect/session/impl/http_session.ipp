//
// http_session.ipp
// ~~~~~~~~~~
//
// http_sessionの実装
//

#ifndef BOOSTCONNECT_SESSION_HTTP_IPP
#define BOOSTCONNECT_SESSION_HTTP_IPP

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
    
http_session::http_session(io_service& io_service): socket_busy_(false),read_timer_(io_service)
{
    socket_ = new bstcon::application_layer::tcp_socket(io_service);
}
#ifdef USE_SSL_BOOSTCONNECT
typedef boost::asio::ssl::context context;
http_session::http_session(io_service& io_service,context& ctx): socket_busy_(false),read_timer_(io_service)
{
    socket_ = new bstcon::application_layer::ssl_socket(io_service,ctx);
}
#endif
http_session::~http_session()
{
    delete socket_;
}

void http_session::start(RequestHandler handler,CloseHandler c_handler)
{
    if(socket_busy_) return; //例外予定
    socket_busy_ = true;
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


void http_session::handle_handshake(const error_code& ec)
{
    if(!ec)
    {
        read_buf_.reset(new boost::asio::streambuf());
        boost::asio::async_read_until(*socket_,*read_buf_.get(),
            "\r\n\r\n",
            boost::bind(&http_session::handle_header_read,shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

        read_timer_.expires_from_now(boost::posix_time::seconds(5));
        read_timer_.async_wait(
            boost::bind(&http_session::read_timeout,shared_from_this(),boost::asio::placeholders::error));
    }
    //else delete this; //例外
}

void http_session::read_timeout(const error_code& ec)
{
    this->end(c_handler_);
    return;
}

void http_session::handle_header_read(const error_code& ec,std::size_t)
{
    if(!ec)
    {
        std::string request_str(boost::asio::buffer_cast<const char*>(read_buf_->data()));

        //リクエストヘッダーのパース
        request_str.erase(0,request_header_parser(request_str,request_));
        request_.body.append(request_str);
        
        size_t byte = int_parser(find_return_or_default(request_.header,"Content-Length","0"));
        if( byte != 0 )
        {
            //長さがある
            read_buf_.reset(new boost::asio::streambuf());

            boost::asio::async_read(
                *socket_,
                *read_buf_.get(),
                boost::asio::transfer_at_least(byte - request_.body.length()),
                boost::bind(&http_session::handle_body_read,shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            //長さがない -> 長さが分からないならこの先読み込むのは危ない
            handle_request_read_complete();
        }
    }
    else std::cout << ec.message() << std::endl;
    //else delete this; //例外
}

void http_session::handle_body_read(const error_code& ec,std::size_t sz)
{
    if(!ec)
    {
        std::string body_str(boost::asio::buffer_cast<const char*>(read_buf_->data()));
        request_.body.append(body_str);

        handle_request_read_complete();
    }
    //else delete this; //例外
}

void http_session::handle_request_read_complete()
{
    keep_alive_ = 
        find_return_or_default(request_.header,"Connection","close") == "Keep-Alive" ||
        find_return_or_default(request_.header,"Proxy-Connection","close") == "Keep-Alive";

    response_type response;
    handler_(request_,response);
    write_buf_.reset(new boost::asio::streambuf());

    //std::ostream os(write_buf_.get()); //TODO:
    std::ostreambuf_iterator<char> out(write_buf_.get());
    if(response_generater(out,response))
    {
        boost::asio::async_write(*socket_,*write_buf_.get(),
            boost::bind(&http_session::handle_write,shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    //else delete this; //パース失敗
}

void http_session::handle_write(const error_code& ec,std::size_t sz)
{
    if(!ec)
    {
        if(keep_alive_)
        {
            handle_handshake(error_code());
        }
        else
        {
            this->end(c_handler_);
            return;
        }
    }
    //else delete this; //例外
}


template<class OutputIterator>
bool http_session::response_generater(OutputIterator& sink,const response_type& response) const
{
    namespace karma = boost::spirit::karma;
        
    bool r = karma::generate(sink,
        "HTTP/" << karma::string << ' ' << karma::int_ << ' ' << karma::string << "\r\n",
        response.http_version,
        response.status_code,
        response.status_message);

    if(!response.header.empty())
    {
        r = r && karma::generate(sink,
            +(karma::string << ": " << karma::string << "\r\n"),
            response.header);
    }

    if(!response.body.empty())
    {
        r = r && karma::generate(sink,
            "\r\n" << karma::string,
            response.body);
    }

    return r;
}

const int http_session::int_parser(const std::string& base) const
{
    namespace qi = boost::spirit::qi;

    int parsed = 0;
    qi::parse(base.cbegin(),base.cend(),qi::int_,parsed);

    return parsed;
}

const int http_session::request_header_parser(const std::string& request_str,bstcon::request& request_containar) const
{
    namespace qi = boost::spirit::qi;

    std::string::const_iterator it = request_str.cbegin();

    qi::parse(
        it,
        request_str.cend(),
        +(qi::char_-' ') >> ' ' >> +(qi::char_-' ') >> ' ' >> +(qi::char_-"\r\n") >> "\r\n",
        request_containar.method, request_containar.path, request_containar.http_version
        );

    qi::parse(
        it,
        request_str.cend(),
        *( +(qi::char_-':') >> ": " >> +(qi::char_-"\r\n") >> "\r\n" ) >> "\r\n",
        request_containar.header
        );

    return std::distance(request_str.cbegin(),it);
}

} // namespace session
} // namespace bstcon

#endif
