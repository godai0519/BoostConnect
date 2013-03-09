//
// multipart_form_data.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_FORM_DATA_IPP
#define BOOSTCONNECT_CONTENT_MULTIPART_FORM_DATA_IPP

#include <string>
#include <boost/foreach.hpp>
#include <boost/random.hpp>
#include "../content_base.hpp"
#include "../multipart_form_data.hpp"

namespace bstcon{
namespace content{
    
multipart_form_data::multipart_form_data(const std::vector<data_set>& data)
    : raw_(data), generated_(false)
{
}

multipart_form_data::~multipart_form_data() // = default;
{
}

std::string multipart_form_data::get_content_type() const
{
    if(boundary_.empty()) refresh_boundary();
    return "multipart/form-data; boundary=" + boundary_;
}
std::string multipart_form_data::get_body() const
{
    if(!generated_)
    {
        if(boundary_.empty()) refresh_boundary();
        data_.clear();
        
        BOOST_FOREACH(const data_set& item ,raw_)
        {
            data_ += "--" + boundary_ + "\r\n";

            data_ += "Content-Disposition: form-data; name=\"" + item.name() + "\"";
            if(!item.filename().empty()) data_ += "; filename=\"" + item.filename() + "\"";
            data_ += "\r\n";

            data_ += "Content-Type: " + item.data()->get_content_type() + "\r\n\r\n";
            data_ += item.data()->get_body() + "\r\n";
        }
        data_ += "--" + boundary_ + "--";

        generated_ = true;
    }

    return data_;
}

void multipart_form_data::refresh_boundary() const
{
    boundary_ = "--------------------" + nonce();
    return;
}

void multipart_form_data::set_data(const std::vector<data_set>& data)
{
    raw_ = data;
    generated_ = false;
    return;
}

inline const std::string multipart_form_data::nonce() const
{
	static const boost::uniform_int<unsigned long> range(0,62-1);
	static boost::mt19937 gen(static_cast<unsigned long>(std::time(0)));
	static boost::variate_generator<
		boost::mt19937&,boost::uniform_int<unsigned long>
	> rand(gen,range);
	static const unsigned char nonce_base[62] = 
	{
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
		'1','2','3','4','5','6','7','8','9','0'
	};

	std::string result;
	unsigned int nonce_length = 0;

	while(nonce_length<12) nonce_length = rand();
	result.reserve(nonce_length);
    
	for(unsigned int i=0;i<nonce_length;++i) result.push_back(nonce_base[rand()]);
	return result;
}

multipart_form_data::data_set::data_set(const boost::shared_ptr<content_base>& data, const std::string& name, const std::string& filename)
    : data_(data), name_(name), filename_(filename)
{
}

multipart_form_data::data_set::~data_set() // = default
{
}

boost::shared_ptr<content_base> multipart_form_data::data_set::data() const
{
    return data_;
}
std::string multipart_form_data::data_set::name() const
{
    return name_;
}
std::string multipart_form_data::data_set::filename() const
{
    return filename_;
}

}
}

#endif
