//
// request.hpp
// ~~~~~~~~~~
//
// Request Containar
//

#ifndef BOOSTCONNECT_REQUEST
#define BOOSTCONNECT_REQUEST

#include <string>
#include <map>
#include <boost/noncopyable.hpp>

namespace bstcon{
    
struct request : boost::noncopyable
{
    typedef std::string                       string_type;
    typedef std::map<string_type,string_type> header_type;
    
    explicit request(const string_type& path = "/", const string_type& body = "")
        : method("GET"), path(path), http_version("1.1"), header(), body(body)
    {
    }

    request(const string_type& method, const string_type& path, const string_type& body = "")
        : method(method), path(path), http_version("1.1"), header(), body(body)
    {
    }
    
    request(const string_type& method, const string_type& path, const string_type& http_version, const header_type& header, const string_type& body = "")
        : method(method), path(path), http_version(http_version), header(header), body(body)
    {
    }
    
    string_type method;
    string_type path;
    string_type http_version;
    header_type header;
    
    string_type body;
};

} // namespace bstcon

#endif
