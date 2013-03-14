//
// text.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_TEXT_HPP
#define BOOSTCONNECT_CONTENT_TEXT_HPP

#include <string>
#include "content_base.hpp"

namespace bstcon{
namespace content{

class text : public content_base
{
public:
    explicit text(const std::string& data, const std::string& type = "text/plain");
    virtual ~text();

    std::string get_content_type() const;
    std::string get_body() const;

    void set_content_type(const std::string& type);
    void set_body(const std::string& data);

private:
    std::string type_;
    std::string data_;
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/text.ipp"
#endif

#endif
