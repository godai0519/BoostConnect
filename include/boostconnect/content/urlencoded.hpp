//
// urlencoded.hpp
// ~~~~~~~~~~
//
// サーバー接続のメイン管理クラス
//

#ifndef BOOSTCONNECT_CONTENT_URLENCODED_HPP
#define BOOSTCONNECT_CONTENT_URLENCODED_HPP

#include <map>
#include <string>
#include "content_base.hpp"
#include "../utility/syntax.hpp"

namespace bstcon{
namespace content{

class urlencoded : public content_base
{
public:
    explicit urlencoded(const std::map<std::string, std::string>& data, const std::string& type = "application/x-www-form-urlencoded");
    virtual ~urlencoded();

    std::string get_content_type() const;
    std::string get_body() const;

    void set_content_type(const std::string& type);
    void set_body(const std::map<std::string, std::string>& data);

private:
    std::string type_;
    std::map<std::string, std::string> data_;

    const bstcon::utility::generator gen_;
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/urlencoded.ipp"
#endif

#endif
