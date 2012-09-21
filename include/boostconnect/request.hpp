//
// request.hpp
// ~~~~~~~~~~
//
// リクエストコンテナー
//

#ifndef BOOSTCONNECT_REQUEST
#define BOOSTCONNECT_REQUEST

#include <string>
#include <map>
#include <boost/noncopyable.hpp>

namespace bstcon{
    
struct request : boost::noncopyable{
    typedef std::string string_type;
    typedef std::map<string_type,string_type> header_type;
    
    string_type method;
    string_type uri;
    string_type http_version;
    header_type header;
    
    string_type body;
};

struct http_request : request{
    typedef std::string string_type;
    typedef std::map<string_type,string_type> header_type;
    
    string_type method;
    string_type uri;
    string_type http_version;
    header_type header;
    
    string_type body;

};

} // namespace bstcon

#endif
