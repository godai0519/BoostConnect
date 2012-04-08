//
// error_code.hpp
// ~~~~~~~~~~
//
// ó·äOèàóùÇ∆Ç©
//

#ifndef TWIT_LIB_SYSTEM_ERROR_CODE
#define TWIT_LIB_SYSTEM_ERROR_CODE

#include <stdexcept>
#include <boost/noncopyable.hpp>
#include <boost/exception/all.hpp>

namespace oauth{
namespace system{
  
namespace error
{
  enum error_t
  {
    success = 0,
    busy
  };
}

struct error_category : public boost::noncopyable{
  virtual ~error_category(){}

  virtual const char* name() const = 0;
  virtual const char* message(int error_code) const = 0;
  
  bool operator==(const error_category& rhs) const { return this == &rhs; }
  bool operator!=(const error_category& rhs) const { return this != &rhs; }
  bool operator<(const error_category& rhs) const { return std::less<const error_category*>()( this, &rhs ); }
};

namespace detail{
  struct system_category : public error_category{
    const char* name() const
    {
      return "system";
    }
    const char* message(int value) const
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
  };
  struct client_category : public error_category{
    const char* name() const
    {
      return "client.socket";
    }
    const char* message(int value) const
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
  };
}

static const error_category& system_category = detail::system_category();
static const error_category& client_category = detail::client_category();

class error_code{
public:
  error_code() : value_(0), category_(&system_category){}
  error_code(int value,const error_category& category) : value_(value), category_(&category){}

  void clear()
  {
    value_ = 0;
    category_ = &system_category;
  }
  void assign(int value,const error_category& category)
  {
    value_ = value;
    category_ = &category;
  }

  //Getter
  int value() const                      { return value_; }
  const error_category& category() const { return *category_; }
  const char* message() const            { return category_->message(value_); }

  //Operator Overwrite
  bool operator!() const {
    return value_ == 0;
  }
  inline friend bool operator==(const error_code &lhs, const error_code &rhs){
    return lhs.category_ == rhs.category_ && lhs.value_ == rhs.value_;
  }
  inline friend bool operator< (const error_code &lhs, const error_code &rhs){
    return lhs.category_ < rhs.category_ || (lhs.category_ == rhs.category_ && lhs.value_ < rhs.value_);
  }

private:
  int                   value_;
  const error_category* category_;
};

typedef boost::error_info<struct tag_errno, int> errno_info;
typedef boost::error_info<struct tag_errcategory, const error_category&> category_info;
typedef boost::error_info<struct tag_errmessage, const char*> message_info;

class exception : public boost::exception, public std::runtime_error {
public:
  exception(error_code& ec, const std::string& what_arg="")
    : std::runtime_error(what_arg), error_code_(ec)
  {
    *this
      << errno_info(ec.value())
      << category_info(ec.category())
      << message_info(ec.message());
  }
  virtual ~exception() throw() {}
 
  const error_code& code() const throw()
  {
    return error_code_;
  }
  const char*       what() const throw()
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
private:
  error_code error_code_;
  mutable std::string what_;
};

void inline throw_error(error_code& ec, const std::string& what_arg=""){
  throw exception(ec,what_arg);
}

} // namespace system
} // namespace oauth

#endif
