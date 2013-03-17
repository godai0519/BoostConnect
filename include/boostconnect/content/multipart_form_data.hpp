//
// multipart_form_data.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_FORM_DATA_HPP
#define BOOSTCONNECT_CONTENT_MULTIPART_FORM_DATA_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "content_base.hpp"
#include "multipart.hpp"

namespace bstcon{
namespace content{

struct multipart_form_data : public multipart
{
    static data_set make_data(
        const boost::shared_ptr<content_base>& data,
        const std::string& name, const std::string& filename="",
#ifdef _MSC_VER
        std::map<std::string, std::string> header = std::map<std::string, std::string>()
#else
        std::map<std::string, std::string> header = {}
#endif
        );

    explicit multipart_form_data(const std::vector<data_set>& data);
    virtual ~multipart_form_data();
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/multipart_form_data.ipp"
#endif

#endif
