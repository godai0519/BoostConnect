//
// http.hpp
// ~~~~~~~~~~
//
// Usually Parser/Generator for HTTP Protocol
//

#ifndef BOOSTCONNECT_UTILITY_HTTP_HPP
#define BOOSTCONNECT_UTILITY_HTTP_HPP

#include <boostconnect/request.hpp>
#include <boostconnect/response.hpp>

namespace bstcon{
namespace utility{

struct http_parser
{
    bool parse_request(bstcon::request& out, std::string const& raw) const;
    bool parse_request_line(bstcon::request& out, std::string const& raw) const;
    bool parse_request_header(bstcon::request& out, std::string const& raw) const;

    bool parse_response(bstcon::response& out, std::string const& raw) const;
    bool parse_response_line(bstcon::response& out, std::string const& raw) const;
    bool parse_response_header(bstcon::response& out, std::string const& raw) const;
};

struct http_generator
{
    bool generate_request(std::ostream& out, bstcon::request const& source) const;
    bool generate_response(std::ostream& out, bstcon::response const& source) const;
};

} // namespace utility
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/http.ipp"
#endif

#endif
