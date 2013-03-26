//
// urlencoded.ipp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_URLENCODED_IPP
#define BOOSTCONNECT_CONTENT_URLENCODED_IPP

#include <map>
#include <string>
#include "../content_base.hpp"
#include "../urlencoded.hpp"

namespace bstcon{
namespace content{

urlencoded::urlencoded(const std::map<std::string, std::string>& data, const std::string& type)
    : content_base(type), data_(data)
{
}
urlencoded::~urlencoded()
{
}

std::string urlencoded::get_body() const
{
    return gen_.urlencode(data_);
}

void urlencoded::set_body(const std::map<std::string, std::string>& data)
{
    data_ = data;
}

}
}

#endif
