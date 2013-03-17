//
// session_base.ipp
// ~~~~~~~~~~
//
// session_baseの実装
//

#ifndef BOOSTCONNECT_SESSION_BASE_IPP
#define BOOSTCONNECT_SESSION_BASE_IPP

#include "../session_base.hpp"

namespace bstcon{
namespace session{
    
session_base::session_base()
{
}
session_base::~session_base()
{
}

inline const std::string& session_base::find_return_or_default(const std::map<std::string,std::string>& map,const std::string& elements,const std::string& defaults) const
{
    return (map.find(elements)==map.cend() ? defaults : map.at(elements));
}

template<class Derived>
session_common<Derived>::session_common()
{
}
template<class Derived>
session_common<Derived>::~session_common()
{
}

} // namespace session
} // namespace bstcon

#endif
