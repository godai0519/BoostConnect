//
// urlencode.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_BINARY_IPP
#define BOOSTCONNECT_CONTENT_BINARY_IPP

#include <string>
#include <vector>
#include "../content_base.hpp"
#include "../binary.hpp"

namespace bstcon{
namespace content{

binary::binary(const std::string& data, const std::string& type)
    : content_base(type), data_(data)
{
}
binary::binary(const std::vector<char>& data, const std::string& type)
    : content_base(type), data_(data.cbegin(), data.cend())
{
}
binary::~binary()
{
}

std::string binary::get_body() const
{
    return data_;
}
void binary::set_body(const std::string& data)
{
    this->data_ = data;
}
void binary::set_body(const std::vector<char>& data)
{
    this->data_.assign(data.cbegin(), data.cend());
}

}
}

#endif
