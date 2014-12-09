//
// multipart_mixed.hpp
// ~~~~~~~~~~
//
// For multipart/mixed Data
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_MIXED_IPP
#define BOOSTCONNECT_CONTENT_MULTIPART_MIXED_IPP

#include <string>
#include "../content_base.hpp"
#include "../multipart.hpp"
#include "../multipart_mixed.hpp"

namespace bstcon{
namespace content{

// auto multipart_mixed::make_data(~) -> data_set{}
multipart_mixed::data_set multipart_mixed::make_data(
    const boost::shared_ptr<content_base>& data,
    std::map<std::string, std::string> header
    )
{
    return data_set(data, header);
}

multipart_mixed::multipart_mixed(const std::vector<data_set>& data)
    : multipart("mixed", data)
{
}
multipart_mixed::~multipart_mixed() // = default;
{
}

}
}

#endif
