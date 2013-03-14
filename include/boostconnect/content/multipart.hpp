//
// multipart.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_CONTENT_MULTIPART_HPP
#define BOOSTCONNECT_CONTENT_MULTIPART_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include "content_base.hpp"

namespace bstcon{
namespace content{

class multipart : public content_base
{
public:
    class data_set
    {
    public:
        explicit data_set(const boost::shared_ptr<content_base>& data, const std::map<std::string, std::string>& header = std::map<std::string, std::string>());
        virtual ~data_set();

        std::map<std::string, std::string> header() const;
        boost::shared_ptr<content_base> data() const;

    private:
        std::map<std::string, std::string> header_;
        boost::shared_ptr<content_base> data_;
    };

    explicit multipart(const std::string& multipart_type, const std::vector<data_set>& data);
    virtual ~multipart();
    
    std::string get_content_type() const;
    std::string get_body() const;
    
    void set_multipart_type(const std::string& multipart_type);
    void set_data(const std::vector<data_set>& data);
    
protected:
    inline const std::string nonce() const;
    void refresh_boundary() const;
    
private:
    mutable std::string data_;
    mutable std::string boundary_;
    mutable bool generated_;
    
    std::vector<data_set> raw_;
    std::string multipart_type_;
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/multipart.ipp"
#endif

#endif
