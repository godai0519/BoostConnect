﻿//
// session_base.hpp
// ~~~~~~~~~~
//
// Provide Connection's Session for HTTP(etc.)
//

#ifndef BOOSTCONNECT_SESSION_BASE_HPP
#define BOOSTCONNECT_SESSION_BASE_HPP

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include "../request.hpp"
#include "../response.hpp"

namespace bstcon{
namespace session{
    
struct session_base : boost::noncopyable
{
    typedef bstcon::request request_type;
    typedef bstcon::response response_type;
    typedef boost::function<void(const request_type&,response_type&)> RequestHandler;
    typedef boost::function<void(boost::shared_ptr<session_base>)> CloseHandler;

    session_base();
    virtual ~session_base();

    virtual void start(RequestHandler handler,CloseHandler c_handler) = 0;
    virtual void end(CloseHandler c_handler) = 0;

    inline const std::string& find_return_or_default(const std::map<std::string,std::string>& map,const std::string& elements,const std::string& defaults) const;
};

template<class Derived>
struct session_common : public session_base, public boost::enable_shared_from_this<Derived>
{
    session_common();
    virtual ~session_common();
};

} // namespace session
} // namespace bstcon


#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/session_base.ipp"
#endif

#endif
