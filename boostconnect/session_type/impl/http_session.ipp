//
// http_session.ipp
// ~~~~~~~~~~
//
// Management a HTTP Connection's Session
//

#ifndef BOOSTCONNECT_SESSION_HTTP_IPP
#define BOOSTCONNECT_SESSION_HTTP_IPP

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boostconnect/session_type/http_session.hpp>
#include <boostconnect/application_layer/tcp_socket.hpp>
#include <boostconnect/application_layer/ssl_socket.hpp>

namespace bstcon{
namespace session{

http_session::http_session(io_service_ptr const& io_service, connection_ptr const& connection, unsigned int const timeout_second, RequestHandler const request_handler, CloseHandler const close_handler)
    : io_service_(io_service), connection_(connection), request_handler_(request_handler), close_handler_(close_handler), timeout_second_(timeout_second), parser_(), generator_()
{
}

void http_session::read()
{
    auto const self = shared_from_this();
    connection_->read_until(
        "\r\n\r\n",
        [this, self](std::string const& header_string)
        {
            //timer_.cancel();

            auto const request = boost::make_shared<bstcon::request>();
            parser_.parse_request(*request, header_string);

            std::size_t size = 0;
            bool keep_alive = false;

            for(auto const& p : request->header)
            {
                if(boost::iequals(p.first, "Content-Length"))
                {
                    size = std::stoi(p.second);
                    continue;
                }
                if(boost::iequals(p.first, "Connection") || boost::iequals(p.first, "Proxy-Connection"))
                {
                    if(!boost::iequals(p.second, "close")) keep_alive = true;
                    continue;
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
    generator_.generate_response(buffer_stream, *header);
    buffer_stream.write(header->body.c_str(), header->body.size());

    return write_buffer(buffer, handler);
}

std::future<void> http_session::set_headers(boost::shared_ptr<bstcon::response> const& header, WriteHandler const handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream buffer_stream(buffer.get());
    generator_.generate_response(buffer_stream, *header);
    
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
