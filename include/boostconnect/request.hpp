//
// request.hpp
// ~~~~~~~~~~
//
// リクエストコンテナー
//

#ifndef TWIT_LIB_PROTOCOL_REQUEST
#define TWIT_LIB_PROTOCOL_REQUEST

#include <string>
#include <map>
#include <boost/noncopyable.hpp>

namespace oauth{
namespace protocol{

struct request : boost::noncopyable{
	typedef std::string string_type;
	typedef std::map<string_type,string_type> header_type;
	
	string_type method;
	string_type uri;
	string_type http_version;
	header_type header;
	
	string_type body;
};

} // namespace protocol
} // namespace oauth

#endif
