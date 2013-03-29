//
// syntax.hpp
// ~~~~~~~~~~
//
// generator for URL Encoding for HTTP and Authorization Field for HTTP
//

#ifndef BOOSTCONNECT_UTILITY_SYNTAX_HPP
#define BOOSTCONNECT_UTILITY_SYNTAX_HPP

#include <map>
#include <string>
#include "percent_encoder.hpp"

namespace bstcon{
namespace utility{    

class generator
{
public:
    generator();
    virtual ~generator(); // =default;

    std::string operator() (const std::map<std::string,std::string>& data, const std::string& key_value_separator = "=", const std::string& field_separator = "&", const std::string& value_container = "") const;

    std::string urlencode(const std::map<std::string,std::string>& data) const;
    std::string authorization_field(const std::map<std::string,std::string>& data) const;

private:
    const bstcon::utility::percent_encoder encoder_;
};

class parser
{
public:
    parser();
    virtual ~parser(); // =default;

    std::map<std::string,std::string> operator() (const std::string& src, const std::string& key_value_separator = "=", const std::string& field_separator = "&", const std::string& value_container = "") const;
    
    std::map<std::string,std::string> urlencode(const std::string& src) const;
    std::map<std::string,std::string> authorization_field(const std::string& src) const;

private:
    const bstcon::utility::percent_encoder encoder_;
};

} // namespace utility
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/syntax.ipp"
#endif

#endif
