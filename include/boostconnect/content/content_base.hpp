//
// content_base.hpp
// ~~~~~~~~~~
//
// Provide common function for HTTPs body make
//

#ifndef BOOSTCONNECT_CONTENT_BASE_HPP
#define BOOSTCONNECT_CONTENT_BASE_HPP

#include <string>
#include <map>

namespace bstcon{
namespace content{

class content_base
{
public:
    explicit content_base(
        const std::string& type,
#ifdef _MSC_VER
        const std::map<std::string, std::string>& attr = std::map<std::string, std::string>()
#else
        const std::map<std::string, std::string>& attr = {}
#endif
    );
    virtual ~content_base();

    virtual std::string get_content_type() const;
    virtual std::string get_body() const = 0;
    
    std::string get_type() const;
    std::map<std::string, std::string> get_attribute() const;

    void set_type(const std::string& type);
    void set_attribute(const std::map<std::string, std::string>& attr);

private:
    std::string type_;
    std::map<std::string, std::string> attr_;
};

}
}

#endif

