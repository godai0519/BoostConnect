//
// error_code.ipp
// ~~~~~~~~~~
//
// error_codeの実装
//

#ifndef BOOSTCONNECT_SYSTEM_ERROR_CODE_IPP
#define BOOSTCONNECT_SYSTEM_ERROR_CODE_IPP

#include "../error_code.hpp"

namespace bstcon{
namespace system{
    
error_category::error_category()
{
}
error_category::~error_category()
{
}

bool error_category::operator==(const error_category& rhs) const
{
    return this == &rhs;
}
bool error_category::operator!=(const error_category& rhs) const
{
    return this != &rhs;
}
bool error_category::operator<(const error_category& rhs) const
{
    return std::less<const error_category*>()( this, &rhs );
}

namespace detail{
    const char* system_category::name() const
    {
        return "system";
    }
    const char* system_category::message(int value) const
    {
        switch(value){
        case error::success:
            return "success";
        case error::busy:
            return "busy";
        default:
            return "system error";
        }
        //No
    }

    const char* client_category::name() const
    {
        return "client.socket";
    }
    const char* client_category::message(int value) const
    {
        switch(value){
        case error::success:
            return "success";
        case error::busy:
            return "Already Connect";
        default:
            return "client.socket error";
        }
        //No
    }
}

error_code::error_code() : value_(0), category_(&system_category)
{
}
error_code::error_code(int value,const error_category& category) : value_(value), category_(&category)
{
}

void error_code::clear()
{
    value_ = 0;
    category_ = &system_category;
}
void error_code::assign(int value,const error_category& category)
{
    value_ = value;
    category_ = &category;
}

//Getter
int error_code::value() const
{
    return value_;
}
const error_category& error_code::category() const
{
    return *category_;
}
const char* error_code::message() const
{
    return category_->message(value_);
}

//Operator Overwrite
bool error_code::operator!() const
{
    return value_ == 0;
}
inline bool operator==(const error_code &lhs, const error_code &rhs)
{
    return lhs.category_ == rhs.category_ && lhs.value_ == rhs.value_;
}
inline bool operator< (const error_code &lhs, const error_code &rhs)
{
    return lhs.category_ < rhs.category_ || (lhs.category_ == rhs.category_ && lhs.value_ < rhs.value_);
}

exception::exception(error_code& ec, const std::string& what_arg)
    : std::runtime_error(what_arg), error_code_(ec)
{
    *this
        << errno_info(ec.value())
        << category_info(ec.category())
        << message_info(ec.message());
}
exception::~exception() throw()
{
}
 
const error_code& exception::code() const throw()
{
    return error_code_;
}
const char* exception::what() const throw()
{
    if(what_.empty())
    {
        try
        {
            what_ = this->std::runtime_error::what();
            if(!what_.empty()) what_+=": ";
            what_ += error_code_.message();
        }
        catch(...)
        {
            return this->std::runtime_error::what();
        }
    }
    return what_.c_str();
}

void inline throw_error(error_code& ec, const std::string& what_arg)
{
    throw exception(ec,what_arg);
}

} // namespace system
} // namespace bstcon

#endif
