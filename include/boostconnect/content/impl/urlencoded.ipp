//
// urlencode.hpp
// ~~~~~~~~~~
//
// サーバー接続のメイン管理クラス
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
    : data_(data), type_(type)
{
}
urlencoded::~urlencoded()
{
}

std::string urlencoded::get_content_type() const
{
    return type_;
}
std::string urlencoded::get_body() const
{
    return gen_.urlencode(data_);
}

void urlencoded::set_content_type(const std::string& type)
{
    this->type_ = type;
}
void urlencoded::set_body(const std::map<std::string, std::string>& data)
{
    this->data_ = data;
}

}
}

#endif
