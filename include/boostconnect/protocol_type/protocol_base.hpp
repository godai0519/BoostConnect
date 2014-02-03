//
// protocol_base.hpp
// ~~~~~~~~~~
//
// Provide HTTP/FTP... Service Common Class powered by Boost.Asio
//

#ifndef BOOSTCONNECT_PROTOCOLTYPE_PROTOCOL_BASE_HPP
#define BOOSTCONNECT_PROTOCOLTYPE_PROTOCOL_BASE_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "../connection_type/connection_base.hpp"

namespace bstcon{
namespace protocol_type{

class protocol_base : boost::noncopyable{
public:
	typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

    protocol_base() {};
    virtual ~protocol_base() {};

	connection_ptr raw_connection(){ return connection_; }

protected:
	connection_ptr connection_;
};

template <class Devide>
class protocol_common : public protocol_base, public boost::enable_shared_from_this<Devide>{};

} // namespace protocol_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/protocol_base.ipp"
#endif

#endif
