#ifndef BOOSTCONNECT_UTILITY_SYNTAX_IPP
#define BOOSTCONNECT_UTILITY_SYNTAX_IPP

#include <map>
#include <string>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include "../syntax.hpp"

namespace bstcon{
namespace utility{    

generator::generator()
{
}
generator::~generator() // =default;
{
}

std::string generator::operator() (const std::map<std::string,std::string>& data, const std::string& key_value_separator, const std::string& field_separator, const std::string& value_container) const
{
    namespace karma = boost::spirit::karma;
        
    const karma::rule<std::back_insert_iterator<std::string>, std::string()> key = karma::string;
    const karma::rule<std::back_insert_iterator<std::string>, std::string()> value = karma::lit(value_container) << karma::string << karma::lit(value_container);
    const karma::rule<std::back_insert_iterator<std::string>, std::map<std::string,std::string>::value_type()> field = key << karma::lit(key_value_separator) << value;
    const karma::rule<std::back_insert_iterator<std::string>, std::map<std::string,std::string>()> body = field % karma::lit(field_separator);

    std::string result;
    std::back_insert_iterator<std::string> out(result);
    karma::generate(out, body, data);

    return result;
}

std::string generator::urlencode(const std::map<std::string,std::string>& data) const
{
    std::map<std::string, std::string> encoded_data;
    typedef std::map<std::string, std::string>::const_reference type;
    BOOST_FOREACH(type x, data)
    {
        std::string encoded_key, encoded_value;
        encoder_.encode(x.first.cbegin(),  x.first.cend(),  std::back_insert_iterator<std::string>(encoded_key));
        encoder_.encode(x.second.cbegin(), x.second.cend(), std::back_insert_iterator<std::string>(encoded_value));
        encoded_data.insert(std::make_pair<std::string, std::string>(std::move(encoded_key),std::move(encoded_value)));            
    }
    return (*this)(encoded_data);
}
std::string generator::authorization_field(const std::map<std::string,std::string>& data) const
{
    std::map<std::string, std::string> encoded_data;
    typedef std::map<std::string, std::string>::const_reference type;
    BOOST_FOREACH(type x, data)
    {
        std::string encoded_key, encoded_value;
        encoder_.encode(x.first.cbegin(),  x.first.cend(),  std::back_insert_iterator<std::string>(encoded_key));
        encoder_.encode(x.second.cbegin(), x.second.cend(), std::back_insert_iterator<std::string>(encoded_value));
        encoded_data.insert(std::make_pair<std::string, std::string>(std::move(encoded_key), std::move(encoded_value)));            
    }
    return (*this)(encoded_data, "=", ",", "\"");
}

parser::parser()
{
}
parser::~parser() // =default;
{
}

std::map<std::string,std::string> parser::operator() (const std::string& src, const std::string& key_value_separator, const std::string& field_separator, const std::string& value_container) const
{
    namespace qi = boost::spirit::qi;
                
    const qi::rule<std::string::const_iterator, std::string(), qi::space_type> key = +(qi::char_ - qi::lit(key_value_separator));
    const qi::rule<std::string::const_iterator, std::string(), qi::space_type> value = (qi::lit(value_container) >> +(qi::char_ - qi::lit(value_container)) >> qi::lit(value_container)) | (+(qi::char_ - qi::lit(field_separator)));
    const qi::rule<std::string::const_iterator,std::pair<std::string,std::string>(), qi::space_type> pair = key >> key_value_separator >> value;
    const qi::rule<std::string::const_iterator,std::map<std::string,std::string>(), qi::space_type> rule = pair % qi::lit(field_separator);

    std::map<std::string,std::string> data;
    std::string::const_iterator it = src.cbegin();
    qi::phrase_parse(it, src.cend(), rule, qi::space, data);

    return data;
}
    
std::map<std::string,std::string> parser::urlencode(const std::string& src) const
{
    std::map<std::string, std::string> decoded_data;
    typedef std::map<std::string, std::string>::const_reference type;
    BOOST_FOREACH(type x, (*this)(src))
    {
        std::string decoded_key, decoded_value;
        encoder_.decode(x.first.cbegin(),  x.first.cend(),  std::back_insert_iterator<std::string>(decoded_key));
        encoder_.decode(x.second.cbegin(), x.second.cend(), std::back_insert_iterator<std::string>(decoded_value));
        decoded_data.insert(std::make_pair<std::string, std::string>(std::move(decoded_key), std::move(decoded_value)));            
    }
    return decoded_data;
}
std::map<std::string,std::string> parser::authorization_field(const std::string& src) const
{
    std::map<std::string, std::string> decoded_data;
    typedef std::map<std::string, std::string>::const_reference type;
    BOOST_FOREACH(type x, (*this)(src, "=", ",", "\""))
    {
        std::string decoded_key, decoded_value;
        encoder_.decode(x.first.cbegin(),  x.first.cend(),  std::back_insert_iterator<std::string>(decoded_key));
        encoder_.decode(x.second.cbegin(), x.second.cend(), std::back_insert_iterator<std::string>(decoded_value));
        decoded_data.insert(std::make_pair<std::string, std::string>(std::move(decoded_key), std::move(decoded_value)));            
    }
    return decoded_data;
}

} // namespace utility
} // namespace bstcon

#endif
