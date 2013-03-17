#ifndef BOOSTCONNECT_UTILITY_RADIX_IPP
#define BOOSTCONNECT_UTILITY_RADIX_IPP

#include <string>
#include "../radix.hpp"

namespace bstcon{
namespace utility{

radix_converter::radix_converter()
{
}
radix_converter::~radix_converter()
{
}

int radix_converter::hex_to_dec(const std::string& hex) const
{
    int dec = 0;
    for(std::string::const_iterator it = hex.cbegin(); it != hex.cend(); ++it)
    {
        dec *= 16;
        if(*it >= '0' && *it <= '9') dec += ((*it)-'0');
        else if(*it >= 'A' && *it <= 'F') dec += ((*it)-'A'+10);
        else if(*it >= 'a' && *it <= 'f') dec += ((*it)-'a'+10);
        else std::exception();
    }
    return dec;
}

} // namespace utility
} // namespace bstcon

#endif
