//
// response.hpp
// ~~~~~~~~~~
//
// Response Containar
//

#ifndef BOOSTCONNECT_RESPONSE
#define BOOSTCONNECT_RESPONSE

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/fusion/sequence/io/out.hpp>

namespace bstcon{

struct response
{
    typedef std::string string_type;
    typedef std::map<string_type,string_type> header_type;
    
    response(){ end_flag_ = 0; }

    string_type http_version;
    int status_code;
    string_type reason_phrase;
    header_type header;
    string_type body;

    int end_flag_;
};
} // namespace bstcon

#endif
