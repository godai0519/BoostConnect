//
// multipart.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_IPP
#define BOOSTCONNECT_CONTENT_MULTIPART_IPP

#include <string>
#include <boost/foreach.hpp>
#include <boost/random.hpp>
#include "../content_base.hpp"
#include "../multipart.hpp"

namespace bstcon{
namespace content{
    
multipart::multipart(const std::string& multipart_type, const std::vector<data_set>& data)
    : multipart_type_(multipart_type), raw_(data), generated_(false)
{
}

multipart::~multipart() // = default;
{
}

std::string multipart::get_content_type() const
{
    if(boundary_.empty()) refresh_boundary();
    return "multipart/" + multipart_type_ + "; boundary=\"" + boundary_ + "\"";
}
std::string multipart::get_body() const
{
    if(!generated_)
    {
        if(boundary_.empty()) refresh_boundary();
        data_.clear();
        
        BOOST_FOREACH(const data_set& item ,raw_)
        {
            data_ += "--" + boundary_ + "\r\n";

            typedef std::pair<std::string, std::string> HeaderType;
            BOOST_FOREACH(const HeaderType& header_item, item.header())
            {
                data_ += header_item.first + ": " + header_item.second + "\r\n";
            }

            data_ += "Content-Type: " + item.data()->get_content_type() + "\r\n\r\n";
            data_ += item.data()->get_body() + "\r\n";
        }
        data_ += "--" + boundary_ + "--";

        generated_ = true;
    }

    return data_;
}

void multipart::refresh_boundary() const
{
    boundary_ = "--------------------" + nonce();
    return;
}

void multipart::set_data(const std::vector<data_set>& data)
{
    raw_ = data;
    generated_ = false;
    return;
}

inline const std::string multipart::nonce() const
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

multipart::data_set::data_set(const boost::shared_ptr<content_base>& data, const std::map<std::string, std::string>& header)
: data_(data), header_(header)
{
}

multipart::data_set::~data_set() // = default
{
}

boost::shared_ptr<content_base> multipart::data_set::data() const
{
    return data_;
}
std::map<std::string, std::string> multipart::data_set::header() const
{
    return header_;
}

}
}

#endif
