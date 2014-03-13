//
// content_base.ipp
// ~~~~~~~~~~
//
// Provide common function for HTTP body make
//

#ifndef BOOSTCONNECT_CONTENT_BASE_IPP
#define BOOSTCONNECT_CONTENT_BASE_IPP

#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include "../content_base.hpp"

namespace bstcon{
namespace content{
    
content_base::content_base(const std::string& type, const std::map<std::string, std::string>& attr)
    : type_(type), attr_(attr)
{
}

content_base::~content_base() // = default;
{
}

std::string content_base::get_content_type() const
{
    std::string content_type = type_;

    typedef std::pair<std::string, std::string> AttrType;
    BOOST_FOREACH(const AttrType& item, attr_)
    {
        content_type += "; ";
        content_type += item.first;
        content_type += "=";
        content_type += item.second;
    }

    return content_type;
}
    
std::string content_base::get_type() const
{
    return type_;
}

std::map<std::string, std::string> content_base::get_attribute() const
{
    return attr_;
}

void content_base::set_type(const std::string& type)
{
    type_ = type;
    return;
}

void content_base::set_attribute(const std::map<std::string, std::string>& attr)
{
    attr_ = attr;
    return;
}

}
}

#endif
