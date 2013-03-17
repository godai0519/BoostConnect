//
// urlencode.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_BINARY_HPP
#define BOOSTCONNECT_CONTENT_BINARY_HPP

#include <string>
#include <vector>
#include "content_base.hpp"

namespace bstcon{
namespace content{

class binary : public content_base
{
public:
    explicit binary(const std::string& data, const std::string& type = "application/octet-stream");
    explicit binary(const std::vector<char>& data, const std::string& type = "application/octet-stream");
    virtual ~binary();

    std::string get_body() const;    
    void set_body(const std::string& data);
    void set_body(const std::vector<char>& data);

private:
    std::string data_;
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/binary.ipp"
#endif

#endif
