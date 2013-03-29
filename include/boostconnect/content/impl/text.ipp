//
// text.hpp
// ~~~~~~~~~~
//
// For text/* Data
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
    : content_base(type), data_(data)
{
}

text::~text()
{
}

std::string text::get_body() const
{
    return data_;
}
void text::set_body(const std::string& data)
{
    data_ = data;
}

}
}

#endif
