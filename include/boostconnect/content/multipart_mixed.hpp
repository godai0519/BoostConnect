//
// multipart_mixed.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_MIXED_HPP
#define BOOSTCONNECT_CONTENT_MULTIPART_MIXED_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "content_base.hpp"
#include "multipart.hpp"

namespace bstcon{
namespace content{

struct multipart_mixed : public multipart
{
    static data_set make_data(
        const boost::shared_ptr<content_base>& data,
#ifdef _MSC_VER
        std::map<std::string, std::string> header = std::map<std::string, std::string>()
#else
        std::map<std::string, std::string> header = {}
#endif
        );

    explicit multipart_mixed(const std::vector<data_set>& data);
    virtual ~multipart_mixed();
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/multipart_mixed.ipp"
#endif

#endif
