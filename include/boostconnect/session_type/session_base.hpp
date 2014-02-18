//
// session_base.hpp
// ~~~~~~~~~~
//
// Provide Connection's Session for HTTP(etc.)
//

#ifndef BOOSTCONNECT_SESSION_BASE_HPP
#define BOOSTCONNECT_SESSION_BASE_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boostconnect/connection_type/connection_base.hpp>

namespace bstcon{
namespace session{

class session_base : boost::noncopyable{
public:
	typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

    session_base() = default;
    virtual ~session_base() = default;

	connection_ptr raw_connection(){ return connection_; }

protected:
	connection_ptr connection_;
};

template <class Devide>
class session_common : public session_base, public boost::enable_shared_from_this<Devide>{};

} // namespace session
} // namespace bstcon


#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/session_base.ipp"
#endif

#endif
