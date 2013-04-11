//
// session_base.ipp
// ~~~~~~~~~~
//
// Provide Connection's Session for HTTP(etc.)
//

#ifndef BOOSTCONNECT_SESSION_BASE_IPP
#define BOOSTCONNECT_SESSION_BASE_IPP

#include <string>
#include <future>
#include <boost/format.hpp>
#include "../session_base.hpp"

namespace bstcon{
namespace session{
    
session_base::session_base()
{
}
session_base::~session_base()
{
}

// 第一引数を16進数にして呼び直す
std::future<void> session_base::set_chunk(const int size, const std::string& body)
{
    const std::string size_str = (boost::format("%x") % size).str();
    return set_chunk(size_str, body);
}

bool session_base::equal_without_capital(const std::string& lhs, const std::string& rhs) const
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](char x, char y)->bool{ return std::tolower(x) == std::tolower(y); })
            && lhs.size() == rhs.size();
}

const std::string& session_base::find_return_or_default(const std::map<std::string,std::string>& map,const std::string& elements,const std::string& defaults) const
{
    for(auto it = map.begin(); it != map.end(); ++it)
    {
        if(equal_without_capital(it->first, elements))
        {
            return it->second;
        }
    }
    return defaults;
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
