//
// error_code.hpp
// ~~~~~~~~~~
//
// Provide Error System
//

#ifndef BOOSTCONNECT_SYSTEM_ERROR_CODE_HPP
#define BOOSTCONNECT_SYSTEM_ERROR_CODE_HPP

#include <stdexcept>
#include <boost/noncopyable.hpp>
#include <boost/exception/all.hpp>

namespace bstcon{
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
    error_category();
    virtual ~error_category();

    virtual const char* name() const = 0;
    virtual const char* message(int error_code) const = 0;
    
    bool operator==(const error_category& rhs) const;
    bool operator!=(const error_category& rhs) const;
    bool operator<(const error_category& rhs) const;
};

namespace detail{
    struct system_category : public error_category{
        const char* name() const;
        const char* message(int value) const;
    };
    struct client_category : public error_category{
        const char* name() const;
        const char* message(int value) const;
    };
}

static const error_category& system_category = detail::system_category();
static const error_category& client_category = detail::client_category();

class error_code{
public:
    error_code();
    error_code(int value,const error_category& category);

    void clear();
    void assign(int value,const error_category& category);

    //Getter
    int value() const;
    const error_category& category() const;
    const char* message() const;

    //Operator Overwrite
    bool operator!() const;
    inline friend bool operator==(const error_code &lhs, const error_code &rhs);
    inline friend bool operator< (const error_code &lhs, const error_code &rhs);

private:
    int value_;
    const error_category* category_;
};

typedef boost::error_info<struct tag_errno, int> errno_info;
typedef boost::error_info<struct tag_errcategory, const error_category&> category_info;
typedef boost::error_info<struct tag_errmessage, const char*> message_info;

class exception : public boost::exception, public std::runtime_error {
public:
    exception(error_code& ec, const std::string& what_arg="");
    virtual ~exception() throw();
 
    const error_code& code() const throw();
    const char* what() const throw();
private:
    error_code error_code_;
    mutable std::string what_;
};

void inline throw_error(error_code& ec, const std::string& what_arg="");

} // namespace system
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/error_code.ipp"
#endif

#endif
