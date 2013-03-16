//
// urlencoded.hpp
// ~~~~~~~~~~
//
// 
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

    std::string get_body() const;
    void set_body(const std::map<std::string, std::string>& data);

private:
    const bstcon::utility::generator gen_;
    std::map<std::string, std::string> data_;
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/urlencoded.ipp"
#endif

#endif
