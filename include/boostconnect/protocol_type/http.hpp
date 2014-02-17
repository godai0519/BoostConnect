//
// http.hpp
// ~~~~~~~~~~
//
// Provide HTTP Service powered by Boost.Asio
//

#ifndef BOOSTCONNECT_PROTOCOLTYPE_HTTP_HPP
#define BOOSTCONNECT_PROTOCOLTYPE_HTTP_HPP

#include <future>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boostconnect/request.hpp>
#include <boostconnect/response.hpp>
#include <boostconnect/protocol_type/protocol_base.hpp>

namespace bstcon{
namespace protocol_type{

struct http_parser
{
    bool parse_status_line(bstcon::response& out, std::string const& raw) const;
    bool parse_header(bstcon::response& out, std::string const& raw) const;
};

class http : public protocol_common<http>
{
public:
    typedef boost::function<void(boost::shared_ptr<bstcon::response> const&)> RequestHandler;

	http(connection_ptr const& connection);
	virtual ~http() = default;

	std::future<bstcon::response> request(bstcon::request const& request, RequestHandler handler = RequestHandler());

private:
	void read(boost::shared_ptr<std::promise<bstcon::response>> const& promise, RequestHandler handler);    
    void read_standard(boost::shared_ptr<std::promise<bstcon::response>> const& promise, boost::shared_ptr<bstcon::response> const& response, RequestHandler handler);    
    void read_chunk_size(boost::shared_ptr<std::promise<bstcon::response>> const& promise, boost::shared_ptr<bstcon::response> const& response, RequestHandler handler);
    void read_chunk(boost::shared_ptr<std::promise<bstcon::response>> const& promise, boost::shared_ptr<bstcon::response> const& response, std::size_t const sz, RequestHandler handler);

    http_parser parser_;
};

} // namespace protocol_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/http.ipp"
#endif

#endif
