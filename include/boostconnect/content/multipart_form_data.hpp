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

namespace bstcon{
namespace content{

class multipart_form_data : public content_base
{
public:
    class data_set
    {
    public:
        explicit data_set(const boost::shared_ptr<content_base>& data, const std::string& name, const std::string& filename="");
        virtual ~data_set();

        boost::shared_ptr<content_base> data() const;
        std::string name() const;
        std::string filename() const;

    private:
        boost::shared_ptr<content_base> data_;
        std::string name_;
        std::string filename_;
    };

    explicit multipart_form_data(const std::vector<data_set>& data);
    virtual ~multipart_form_data();
    
    std::string get_content_type() const;
    std::string get_body() const;
    
    void set_data(const std::vector<data_set>& data);

private:
    inline const std::string nonce() const;
    void refresh_boundary() const;

    mutable std::string data_;
    mutable std::string boundary_;
    mutable bool generated_;
    
    std::vector<data_set> raw_;
};

}
}

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/multipart_form_data.ipp"
#endif

#endif
