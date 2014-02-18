//
// http.ipp
// ~~~~~~~~~~
//
// Provide HTTP Service powered by Boost.Asio
//

#ifndef BOOSTCONNECT_PROTOCOLTYPE_HTTP_IPP
#define BOOSTCONNECT_PROTOCOLTYPE_HTTP_IPP

#include <boost/algorithm/string.hpp>
#include <boostconnect/protocol_type/http.hpp>

namespace bstcon{
namespace protocol_type{

http::http(connection_ptr const& connection)
    : parser_(), generator_()
{
    connection_ = connection;
    return;
}

std::future<bstcon::response> http::request(bstcon::request const& request, RequestHandler handler)
{
    auto buffer = boost::make_shared<boost::asio::streambuf>();
    std::ostream os(buffer.get());
    generator_.generate_request(os, request);

    auto p = boost::make_shared<std::promise<bstcon::response>>();
    auto self = shared_from_this();
    connection_->write(
        buffer,
        [self, p, buffer, handler](std::size_t const sz) -> void
        {
            self->read(p, handler);
            return;
        }
    );

    return p->get_future();
}

void http::read(boost::shared_ptr<std::promise<bstcon::response>> const& promise, RequestHandler handler)
{
    auto self = this->shared_from_this();
    connection_->read_line(
        [self, promise, handler](std::string const& line)
        {
            auto response = boost::make_shared<bstcon::response>();
            self->parser_.parse_response_line(*response, line);

            self->connection_->read_until(
                "\r\n\r\n",
                [self, promise, response, handler](std::string const& raw_header)
                {
                    self->parser_.parse_response_header(*response, raw_header);

                    bool is_chunked = false;
                    for(auto const& p : response->header)
                    {
                        if(boost::iequals(p.first, "Transfer-Encoding"))
                        {
                            is_chunked = (boost::iequals(p.second, "chunked")) ? true : false;
                            break;
                        }
                    }

                    if(is_chunked) self->read_chunk_size(promise, response, handler);
                    else           self->read_standard  (promise, response, handler);

                    return;
                }
            );

            return;
        }
    );

    return;
}
    
void http::read_standard(boost::shared_ptr<std::promise<bstcon::response>> const& promise, boost::shared_ptr<bstcon::response> const& response, RequestHandler handler)
{
    std::size_t body_size = 0;
    for(auto const& p : response->header)
    {
        if(boost::iequals(p.first, "Content-Length"))
        {
            body_size = std::stoi(p.second);
            break;
        }
    }

    auto self = shared_from_this();
    if(body_size > 0)
    {
        connection_->read_size(
            body_size,
            [self, promise, response, handler](std::string const& body)
            {
                response->body = body;
                if(handler) handler(response);
                promise->set_value(*response);
                return;
            }
        );
    }
    else
    {
        connection_->read(
            [self, promise, response, handler](std::string const& body)
            {
                response->body = body;
                if(handler) handler(response);
                promise->set_value(*response);
                return;
            }
        );
    }

    return;
}
    
void http::read_chunk_size(boost::shared_ptr<std::promise<bstcon::response>> const& promise, boost::shared_ptr<bstcon::response> const& response, RequestHandler handler)
{
    auto self = shared_from_this();
    connection_->read_line(
        [self, promise, response, handler](std::string const& chunk_line)
        {
            self->read_chunk(promise, response, std::stoi(chunk_line, nullptr, 16), handler);
            return;
        }
    );

    return;
}

void http::read_chunk(boost::shared_ptr<std::promise<bstcon::response>> const& promise, boost::shared_ptr<bstcon::response> const& response, std::size_t const sz, RequestHandler handler)
{
    auto self = shared_from_this();
    if(sz == 0)
    {
        connection_->read_size(
            2,
            [self, promise, response, handler](std::string const&)
            {
                if(handler) handler(response);
                promise->set_value(*response);

                return;
            }
        );

        return;
    }

    connection_->read_size(
        sz+2,
        [self, promise, response, handler](std::string const& body)
        {
            response->body += body.substr(0, body.size()-2);
            self->read_chunk_size(promise, response, handler);
            return;
        }
    );
}

} // namespace protocol_type
} // namespace bstcon

#endif
