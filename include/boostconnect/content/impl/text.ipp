//
// text.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_TEXT_IPP
#define BOOSTCONNECT_CONTENT_TEXT_IPP

#include <map>
#include <string>
#include "../content_base.hpp"
#include "../text.hpp"

namespace bstcon{
namespace content{

text::text(const std::string& data, const std::string& type)
        : data_(data), type_(type)
{
}
text::~text()
{
}

std::string text::get_content_type() const
{
    return type_;
}
std::string text::get_body() const
{
    return data_;
}

void text::set_content_type(const std::string& type)
{
    this->type_ = type;
}
void text::set_body(const std::string& data)
{
    this->data_ = data;
}

}
}

#endif
