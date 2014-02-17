//
// http.ipp
// ~~~~~~~~~~
//
// Provide HTTP Service powered by Boost.Asio
//

#ifndef BOOSTCONNECT_PROTOCOLTYPE_HTTP_IPP
#define BOOSTCONNECT_PROTOCOLTYPE_HTTP_IPP

#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/vector_tie.hpp>
#include <boostconnect/protocol_type/http.hpp>

namespace bstcon{
namespace protocol_type{

bool http_parser::parse_status_line(bstcon::response& out, std::string const& raw) const
{
    namespace qi = boost::spirit::qi;
    namespace phx = boost::phoenix;
    typedef boost::fusion::vector<std::string, unsigned int, std::string> tuple_type;
    typedef std::string::const_iterator iterator_type;
        
    qi::rule<iterator_type, std::string()> http_version;
    qi::rule<iterator_type, unsigned int()> status_code;
    qi::rule<iterator_type, std::string()> reason_phrase;
    qi::rule<iterator_type, tuple_type()> status_line;
        
    http_version  %= qi::omit[qi::string("HTTP/")] >> qi::raw[qi::char_("0-9") >> qi::string(".") >> qi::char_("0-9")];
    status_code   %= qi::raw[qi::digit >> qi::digit >> qi::digit];
    reason_phrase %= *(qi::char_ - qi::char_("\r\n"));
    status_line   %= http_version[phx::at_c<0>(qi::_val)] >> " " >> status_code[phx::at_c<1>(qi::_val)] >> " " >> reason_phrase[phx::at_c<2>(qi::_val)] >> qi::lit("\r\n");

    tuple_type parsed;
    bool const result = qi::parse(raw.cbegin(), raw.cend(), status_line, parsed);
    if(result) boost::fusion::vector_tie(out.http_version, out.status_code, out.reason_phrase) = std::move(parsed);

    return result;
}

bool http_parser::parse_header(bstcon::response& out, std::string const& raw) const
{
    namespace qi = boost::spirit::qi;
    namespace phx = boost::phoenix;
    typedef std::map<std::string, std::string> map_type;
    typedef std::string::const_iterator iterator_type;
        
    qi::rule<iterator_type, std::pair<std::string, std::string>(), qi::space_type> header_pair;
    qi::rule<iterator_type, map_type(), qi::space_type> header;
        
    header_pair %= qi::lexeme[*(qi::char_ - qi::char_(':'))] >> qi::lit(':') >> qi::lexeme[*(qi::char_ - qi::eol)];
    header %= *header_pair[phx::insert(qi::_val, qi::_1)];
        
    map_type parsed;
    bool const result = qi::phrase_parse(raw.cbegin(), raw.cend(), header, qi::space, parsed);
    if(result) std::swap(out.header, std::move(parsed));

    return result;
}

http::http(connection_ptr const& connection)
{
	connection_ = connection;
	return;
}

std::future<bstcon::response> http::request(bstcon::request const& request, RequestHandler handler)
{
	auto buffer = boost::make_shared<boost::asio::streambuf>();
	std::ostream os(buffer.get());
		
	os.write(request.method.c_str(), request.method.size());
	os.write(" ", 1);
	os.write(request.path.c_str(), request.path.size());
	os.write(" HTTP/", 6);
	os.write(request.http_version.c_str(), request.http_version.size());
	os.write("\r\n", 2);

	for(auto const& pair : request.header)
	{
		os.write(pair.first.c_str(), pair.first.size());
		os.write(": ", 2);
		os.write(pair.second.c_str(), pair.second.size());
		os.write("\r\n", 2);
	}
		
	os.write("\r\n", 2);
	os.write(request.body.c_str(), request.body.size());

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
            self->parser_.parse_status_line(*response, line);

            self->connection_->read_until(
                "\r\n\r\n",
                [self, promise, response, handler](std::string const& raw_header)
                {
                    self->parser_.parse_header(*response, raw_header);

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
