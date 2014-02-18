//
// http_session.ipp
// ~~~~~~~~~~
//
// Management a HTTP Connection's Session
//

#ifndef BOOSTCONNECT_SESSION_HTTP_IPP
#define BOOSTCONNECT_SESSION_HTTP_IPP

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>


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
    
struct http_parser
{
    static bool parse_request_header(bstcon::request& out, std::string const& raw)
    {
        namespace qi = boost::spirit::qi;

        auto it = raw.cbegin();

        qi::parse(
            it, raw.cend(),
            +(qi::char_-' ') >> ' ' >> +(qi::char_-' ') >> ' ' >> +(qi::char_-"\r\n") >> "\r\n",
            out.method, out.path, out.http_version
            );

        qi::parse(
            it, raw.cend(),
            *( +(qi::char_-':') >> ": " >> +(qi::char_-"\r\n") >> "\r\n" ) >> "\r\n",
            out.header
            );

        return true;
    }
};

struct generate
{
    static bool generate_response_header(std::ostream& out, bstcon::response const& source)
    {
        out.write("HTTP/", 5);
        out.write(source.http_version.c_str(), source.http_version.size());
        out.write(" ", 1);
        out.write(std::to_string(source.status_code).c_str(), 3);
        out.write(" ", 1);
        out.write(source.reason_phrase.c_str(), source.reason_phrase.size());
        out.write("\r\n", 2);

        for(auto const& pair : source.header)
        {
            out.write(pair.first.c_str(), pair.first.size());
            out.write(": ", 2);
            out.write(pair.second.c_str(), pair.second.size());
            out.write("\r\n", 2);
        }
        out.write("\r\n", 2);
        return true;
    }
};


http_session::http_session(io_service_ptr const& io_service, connection_ptr const& connection, unsigned int const timeout_second, RequestHandler const request_handler, CloseHandler const close_handler)
    : io_service_(io_service), connection_(connection), timeout_second_(timeout_second), request_handler_(request_handler), close_handler_(close_handler)
{
}

//
//    
//http_session::http_session(io_service& io_service, unsigned int timeout_second)
//    : read_timer_(io_service), socket_(new bstcon::application_layer::tcp_socket(io_service)), timeout_second_(timeout_second)
//{
//}
//#ifdef USE_SSL_BOOSTCONNECT
//typedef boost::asio::ssl::context context;
//http_session::http_session(io_service& io_service, context& ctx, unsigned int timeout_second)
//    : read_timer_(io_service), socket_(new bstcon::application_layer::ssl_socket(io_service,ctx)), timeout_second_(timeout_second)
//{
//}
//#endif

//void http_session::end(CloseHandler c_handler)
//{
//    socket_->close();
//
//    c_handler(static_cast<boost::shared_ptr<session_base>>(shared_from_this()));
//    return;
//}

void http_session::read()
{
    auto const self = shared_from_this();
    connection_->read_until(
        "\r\n\r\n",
        [this, self](std::string const& header_string)
        {
            //timer_.cancel();

            auto const request = boost::make_shared<bstcon::request>();
            http_parser::parse_request_header(*request, header_string);

            std::size_t size = 0;
            bool keep_alive = false;
            for(auto const& p : request->header)
            {
                if(boost::iequals(p.first, "Content-Length"))
                {
                    size = std::stoi(p.second);
                    break;
                }
                if(boost::iequals(p.first, "Connection") || boost::iequals(p.first, "Proxy-Connection"))
                {
                    if(!boost::iequals(p.second, "close")) keep_alive = true;
                    break;
                }
            }

            if(size > 0)
            {
                connection_->read_size(
                    size,
                    [this, self, request, keep_alive](std::string const& body)
                    {
                        if(keep_alive) read();

                        request->body = body;
                        if(request_handler_) request_handler_(request, self);

                        return;
                    }
                );
            }
            else
            {
                // Non Content-Length == No Body
                if(keep_alive) read();
                if(request_handler_) request_handler_(request, self);
            }

            return;
        }
    );

    //timer_.expires_from_now(boost::posix_time::seconds(timeout_second_));
    //timer_.async_wait(
    //    [this, self](boost::system::error_code const& ec)
    //    {
    //        close();
    //        return;
    //    }
    //);

    return;
}

void http_session::close()
{
    connection_->close();
    if(close_handler_) close_handler_(shared_from_this());

    return;
}

std::future<void> http_session::set_all(boost::shared_ptr<bstcon::response> const& header, WriteHandler const handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream buffer_stream(buffer.get());
    generate::generate_response_header(buffer_stream, *header);
    buffer_stream.write(header->body.c_str(), header->body.size());

    return write_buffer(buffer, handler);
}

std::future<void> http_session::set_headers(boost::shared_ptr<bstcon::response> const& header, WriteHandler const handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream buffer_stream(buffer.get());
    generate::generate_response_header(buffer_stream, *header);
    
    return write_buffer(buffer, handler);
}

std::future<void> http_session::set_body(std::string const& body, WriteHandler const handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream buffer_stream(buffer.get());
    buffer_stream.write(body.c_str(), body.size());

    return write_buffer(buffer, handler);
}

std::future<void> http_session::set_chunk(WriteHandler const handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream buffer_stream(buffer.get());
    buffer_stream.write("0\r\n\r\n", 5);

    return write_buffer(buffer, handler);
}

std::future<void> http_session::set_chunk(std::string const& chunk, WriteHandler const handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream buffer_stream(buffer.get());
    buffer_stream << boost::format("%03x\r\n") % chunk.size();
    buffer_stream.write(chunk.c_str(), chunk.size());
    buffer_stream.write("\r\n", 2);

    return write_buffer(buffer, handler);
}

std::future<void> http_session::write_buffer(boost::shared_ptr<boost::asio::streambuf> const& buffer, WriteHandler const handler)
{
    auto self = shared_from_this();
    auto promise = boost::make_shared<std::promise<void>>();
    connection_->write(buffer,
        [handler, promise, buffer, self](std::size_t const size)
        {
            if(handler) handler(self);
            promise->set_value();
            return;
        }
    );

    return promise->get_future();
}

} // namespace session
} // namespace bstcon

#endif
