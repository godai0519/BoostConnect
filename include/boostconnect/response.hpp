//
// response.hpp
// ~~~~~~~~~~
//
// レスポンスコンテナー
//

#ifndef TWIT_LIB_PROTOCOL_RESPONSE
#define TWIT_LIB_PROTOCOL_RESPONSE

#include <string>
#include <map>
#include <boost/noncopyable.hpp>

namespace oauth{
namespace protocol{

struct response : boost::noncopyable{
	typedef std::string string_type;
	typedef std::map<string_type,string_type> header_type;
	
	int status_code;
	string_type http_version;
	string_type status_message;
	header_type header;
	string_type body;

	response() : status_code(0){}
};

} // namespace protocol
} // namespace oauth

#endif
