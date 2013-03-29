//
// radix.hpp
// ~~~~~~~~~~
//
// Radix Converter
//

#ifndef BOOSTCONNECT_UTILITY_RADIX_HPP
#define BOOSTCONNECT_UTILITY_RADIX_HPP

#include <string>

namespace bstcon{
namespace utility{

class radix_converter
{
public:
    radix_converter();
    virtual ~radix_converter();

    int hex_to_dec(const std::string& hex) const;
};

} // namespace utility
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/radix.ipp"
#endif

#endif
