//
// content_base.hpp
// ~~~~~~~~~~
//
// サーバー接続のメイン管理クラス
//

#ifndef BOOSTCONNECT_CONTENT_BASE_HPP
#define BOOSTCONNECT_CONTENT_BASE_HPP

#include <string>

namespace bstcon{
namespace content{

struct content_base
{
    virtual std::string get_content_type() const = 0;
    virtual std::string get_body() const = 0;
};

}
}

#endif
