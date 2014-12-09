//
// multipart_form_data.hpp
// ~~~~~~~~~~
//
// For multipart/form_data Data
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_FORM_DATA_IPP
#define BOOSTCONNECT_CONTENT_MULTIPART_FORM_DATA_IPP

#include <string>
#include "../content_base.hpp"
#include "../multipart.hpp"
#include "../multipart_form_data.hpp"

namespace bstcon{
namespace content{

// auto multipart_form_data::make_data(~) -> data_set{}
multipart_form_data::data_set multipart_form_data::make_data(
    const boost::shared_ptr<content_base>& data,
    const std::string& name,
    const std::string& filename,
    std::map<std::string, std::string> header
    )
{
    std::string disposition;

    disposition += "form-data";
    if(!name.empty()) disposition += "; name=\"" + name + "\"";
    if(!filename.empty()) disposition += "; filename=\"" + filename + "\"";
    header["Content-Disposition"] = disposition;

    return data_set(data, header);
}

multipart_form_data::multipart_form_data(const std::vector<data_set>& data)
    : multipart("form-data", data)
{
}
multipart_form_data::~multipart_form_data() // = default;
{
}

}
}

#endif
