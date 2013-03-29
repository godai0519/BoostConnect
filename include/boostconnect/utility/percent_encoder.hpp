//
// percent_encoding.hpp
// ~~~~~~~~~~
//
// percent(URL) Encoder
//

#ifndef BOOSTCONNECT_UTILITY_PERCENT_ENCODER_HPP
#define BOOSTCONNECT_UTILITY_PERCENT_ENCODER_HPP

#include <string>
#include "radix.hpp"

namespace bstcon{
namespace utility{

class percent_encoder
{
public:
    percent_encoder();
    virtual ~percent_encoder();// = default;

    template<typename InputIterator, typename OutputIterator>
    OutputIterator encode(InputIterator first, InputIterator last, OutputIterator out) const;

    template<typename InputIterator, typename OutputIterator>
    OutputIterator decode(InputIterator first, InputIterator last, OutputIterator out) const;

private:
    const bstcon::utility::radix_converter radix_;
};

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/percent_encoder.ipp"
#endif

} // namespace utility
} // namespace bstcon

#endif
