//
// http.ipp
// ~~~~~~~~~~
//
// Usually Parser/Generator for HTTP Protocol
//

#ifndef BOOSTCONNECT_UTILITY_HTTP_IPP
#define BOOSTCONNECT_UTILITY_HTTP_IPP

#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/vector_tie.hpp>
#include <boostconnect/utility/http.hpp>

namespace bstcon{
namespace utility{

bool http_parser::parse_request(bstcon::request& out, std::string const& raw) const
{
    auto const sz = raw.find("\r\n");

    bool success = true;
    success &= parse_request_line  (out, raw.substr(0, sz+2));
    success &= parse_request_header(out, raw.substr(sz+2   ));
    
    return success;
}

bool http_parser::parse_request_line(bstcon::request& out, std::string const& raw) const
{
    namespace qi = boost::spirit::qi;

    auto it = raw.cbegin();
    qi::parse(
        it, raw.cend(),
        +(qi::char_-' ') >> ' ' >> +(qi::char_-' ') >> ' ' >> +(qi::char_-"\r\n") >> "\r\n",
        out.method, out.path, out.http_version
        );

    return true;
}

bool http_parser::parse_request_header(bstcon::request& out, std::string const& raw) const
{
    namespace qi = boost::spirit::qi;

    auto it = raw.cbegin();
    qi::parse(
        it, raw.cend(),
        *( +(qi::char_-':') >> ": " >> +(qi::char_-"\r\n") >> "\r\n" ) >> "\r\n",
        out.header
        );

    return true;
}

bool http_parser::parse_response(bstcon::response& out, std::string const& raw) const
{
    auto const sz = raw.find("\r\n");

    bool success = true;
    success &= parse_response_line  (out, raw.substr(0, sz+2));
    success &= parse_response_header(out, raw.substr(sz+2   ));

    return success;
}

bool http_parser::parse_response_line(bstcon::response& out, std::string const& raw) const
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

bool http_parser::parse_response_header(bstcon::response& out, std::string const& raw) const
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
    if(result) std::swap(out.header, parsed);

    return result;
}

bool http_generator::generate_request(std::ostream& out, bstcon::request const& source) const
{
    out.write(source.method.c_str(), source.method.size());
    out.write(" ", 1);
    out.write(source.path.c_str(), source.path.size());
    out.write(" HTTP/", 6);
    out.write(source.http_version.c_str(), source.http_version.size());
    out.write("\r\n", 2);

    for(auto const& pair : source.header)
    {
        out.write(pair.first.c_str(), pair.first.size());
        out.write(": ", 2);
        out.write(pair.second.c_str(), pair.second.size());
        out.write("\r\n", 2);
    }

    out.write("\r\n", 2);
    out.write(source.body.c_str(), source.body.size());
    return true;
}

bool http_generator::generate_response(std::ostream& out, bstcon::response const& source) const
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

} // namespace utility
} // namespace bstcon

#endif
