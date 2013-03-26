#ifndef BOOSTCONNECT_UTILITY_PERCENT_ENCODER_IPP
#define BOOSTCONNECT_UTILITY_PERCENT_ENCODER_IPP

#include <boost/format.hpp>
#include "../percent_encoder.hpp"
#include "../radix.hpp"

namespace bstcon{
namespace utility{

percent_encoder::percent_encoder()
{
}
percent_encoder::~percent_encoder() // = default;
{
}

template<typename InputIterator, typename OutputIterator>
OutputIterator percent_encoder::encode(InputIterator first, InputIterator last, OutputIterator out) const
{
    while(first != last)
    {
        if((0x30<=*first && *first<=0x39) || (0x41<=*first && *first<=0x5a) || (0x61<=*first && *first<= 0x7a) || *first==0x2d || *first==0x2e || *first==0x5f || *first==0x7e)
        {
            *out++ = *first++;
        }
        else
        {
            std::string ss((boost::format("%%%02X") % (*first++ & 0xff)).str());
            out = std::copy(ss.begin(), ss.end(), out);
        }
    }
    return out;
}

template<typename InputIterator, typename OutputIterator>
OutputIterator percent_encoder::decode(InputIterator first, InputIterator last, OutputIterator out) const
{
    while(first != last)
    {
        if(*first == '%') //%xx
        {
            *out++ = (char)radix_.hex_to_dec(std::string(first+1,first+3));
            first+=3;
        }
        else *out++ = *first++;
    }
    return out;
}

} // namespace utility
} // namespace bstcon

#endif
