//
// connection_base.hpp
// ~~~~~~~~~~
//
// 同期や非同期判断のための、Boost.Asioを使用したクラス群
//

#ifndef TWIT_LIB_PROTOCOL_CONNECTTYPE_CONNECTION_BASE
#define TWIT_LIB_PROTOCOL_CONNECTTYPE_CONNECTION_BASE

#include <memory>
#include <boost/asio.hpp>
#include "../application_layer/layer_base.hpp"

namespace oauth{
namespace protocol{
namespace connection_type{
	enum connection_type{async,sync};

class connection_base : boost::noncopyable{
public:
	typedef boost::system::error_code error_code;

	connection_base(){}
	virtual ~connection_base(){}

	//追加	
	virtual error_code& operator() (const std::string&,boost::asio::streambuf&,error_code&) = 0;
	virtual error_code& operator() (
		boost::asio::ip::tcp::resolver::iterator ep_iterator,
		boost::asio::streambuf& buf,
		error_code& ec
		) = 0;
protected:
	std::shared_ptr<application_layer::layer_base> socket_layer_;
};

} // namespace connection_type
} // namespace protocol
} // namespace oauth

#endif
