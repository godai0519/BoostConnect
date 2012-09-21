//
// session_base.hpp
// ~~~~~~~~~~
//
// Sessionã§í ê›íË
//

#ifndef BOOSTCONNECT_SESSION_BASE
#define BOOSTCONNECT_SESSION_BASE

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "../request.hpp"
#include "../response.hpp"

namespace bstcon{
namespace session{
    
struct session_base : boost::noncopyable{
    typedef bstcon::request request_type;
    typedef bstcon::response response_type;
    typedef boost::function<void(const request_type&,response_type&)> RequestHandler;
    typedef boost::function<void(boost::shared_ptr<session_base>)> CloseHandler;

    session_base(){}
    virtual ~session_base(){}

    virtual void start(RequestHandler handler,CloseHandler c_handler) = 0;
    virtual void end(CloseHandler c_handler) = 0;

    inline const std::string& find_return_or_default(const std::map<std::string,std::string>& map,const std::string& elements,const std::string& defaults) const
    {
        return (map.find(elements)==map.cend() ? defaults : map.at(elements));
    }
};

template<class Derived>
struct session_common : public session_base, public boost::enable_shared_from_this<Derived>{
    session_common(){}
    virtual ~session_common(){}
};

} // namespace session
} // namespace bstcon

#endif
