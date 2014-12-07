//
// connection.hpp
// ~~~~~~~~~~
//
// 
//

#ifndef BOOSTCONNECT_HTTP_CONNECTION_HPP
#define BOOSTCONNECT_HTTP_CONNECTION_HPP

#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include "../connection_type/connection_base.hpp"
#include "../request.hpp"
#include "../response.hpp"

namespace bstcon{
namespace http{

template<typename ConnectionType>
class reader;

template<typename ConnectionType>
class connection : private ConnectionType
{
public:
    typedef boost::function<bool(const boost::shared_ptr<bstcon::response>&)> StreamHandler;

    explicit connection(const boost::shared_ptr<application_layer::socket_base>& socket)
        : ConnectionType(socket)
    {
    }

    virtual ~connection()
    {
    }

    using ConnectionType::connect;
    using ConnectionType::close;

    std::future<bstcon::response> request(const boost::shared_ptr<bstcon::request>& request)
    {
        auto buf = boost::make_shared<boost::asio::streambuf>();
        
        // func‰»
        {
            std::ostream os(buf.get());
            os << request->method << " " << request->path << "HTTP/" << request->http_version << "\r\n";
            for(const auto& pair : request->header) os << pair.first << ": " << pair.second << "\r\n";
            os << "\r\n";
            os << request->body;
        }
        // func‰»(‚±‚±‚Ü‚Å)
        
        auto p = boost::make_shared<std::promise<bstcon::response>>();
        ConnectionType::write(buf,
            [p](const std::size_t size)->void
            {
                // read -> response
                // p->set_value(response)
            }
        );

        return p->get_future();
    }

    void streaming(const boost::shared_ptr<bstcon::request>& request, StreamHandler handler)
    {
        auto buf = boost::make_shared<boost::asio::streambuf>();
        
        // func‰»
        {
            std::ostream os(buf.get());
            os << request->method << " " << request->path << "HTTP/" << request->http_version << "\r\n";
            for(const auto& pair : request->header) os << pair.first << ": " << pair.second << "\r\n";
            os << "\r\n";
            os << request->body;
        }
        // func‰»(‚±‚±‚Ü‚Å)

        ConnectionType::write(buf,
            [](const std::size_t size)->void
            {
                // read -> handler
            }
        );

        return;
    }

    //void accept();

private:
};

template<typename ConnectionType>
class reader : public boost::enable_shared_from_this<reader<ConnectionType>>
{
public:
    typedef boost::function<bool(const std::shared_ptr<bstcon::response>&)> Chunkhandler;
    typedef boost::function<void(const std::shared_ptr<bstcon::response>&)> Endhandler;

    typedef boost::system::error_code                        error_code;
    typedef boost::shared_ptr<bstcon::response>              response_type;
    typedef boost::function<bool(response_type, error_code)> ChunkHandler;
    typedef boost::function<void(error_code)>                EndHandler;

    explicit reader(const boost::shared_ptr<ConnectionType>& connection)
        : connection_(connection)
    {
    }

    virtual ~reader()
    {
    }

    void operator() (ChunkHandler chandler, EndHandler ehandler)
    {
        chandler_ = chandler;
        ehandler_ = ehandler;
        response_ = boost::make_shared<bstcon::response>();
        header_read();
        return;
    }

private:
    boost::optional<std::string> get_headers_value(const std::string& key) const
    {
        for(const std::pair<std::string, std::string>& p : response_->header)
        {
            if(boost::algorithm::iequals(p.first, key)) return p.second;
        }
        return boost::none;
    }

    int parse_header(const std::string& source)
    {        
        namespace qi = boost::spirit::qi;

        qi::rule<std::string::const_iterator,std::pair<std::string,std::string>> field = (+(qi::char_ - ": ") >> ": " >> +(qi::char_ - "\r\n") >> "\r\n");
            
        std::string::const_iterator it = source.cbegin();
        qi::parse(it,source.cend(),"HTTP/" >> +(qi::char_ - " ") >> " " >> qi::int_ >> " " >> +(qi::char_ - "\r\n") >> ("\r\n"),
            response_->http_version,
            response_->status_code,
            response_->status_message);

        qi::parse(it,source.cend(),*(+(qi::char_ - ": ") >> ": " >> +(qi::char_ - "\r\n") >> "\r\n"),
            response_->header);

        qi::parse(it,source.cend(),qi::lit("\r\n"));
        
        return std::distance(source.cbegin(),it);
    }

    void header_read()
    {
        connection_->read_until("\r\n\r\n",
            [this](const std::string& header_str)
            {
                parse_header(header_str);

                if(chandler_) chandler_(response_);
                
                const auto transfer_encoding = get_headers_value("Transfer-Encoding");
                if(boost::iequals(transfer_encoding.get(), "chunked"))
                {
                    chunk_size_read();
                }
                else
                {
                    size_t body_size = 0;
                    const auto content_length = get_headers_value("Content-Length");
                    if(content_length) body_size = std::atoi(content_length.get().c_str());

                    if(body_size > 0)
                    {
                        size_read(body_size);
                    }
                    else
                    {
                        //‚Ç‚¤‚µ‚æ‚¤‚©H
                    }
                }

                return;
            }
        );
        return;
    }

    void chunk_size_read()
    {
        connection_->read_line(
            [this](const std::string& line)
            {
                const size_t size = std::atoi(line.c_str());

                if(size == 0)
                {
                    //read_end

                    if(ehandler) ehandler(response_);
                    return;
                }

                this->chunk_read(size);
                return;
            }
        );

        return;
    }

    void chunk_read(const size_t size)
    {
        connection->read_size(size,
            [this](const std::string& body)
            {
                response_->body.append(body);                
                if(chandler) chandler(response_);

                this->chunk_size_read();
                return;
            }
        );
        return;
    }

    void size_read(const size_t size)
    {
        connection->read_size(size,
            [this](const std::string& body)
            {
                // TODO: Use std::swap
                response_->body = body;

                if(ehandler) ehandler(response_);

                return;
            }
        );
        return;
    }

    ChunkHandler chandler_;
    EndHandler   ehandler_;

    boost::shared_ptr<bstcon::response> response_;
    const boost::shared_ptr<ConnectionType> connection_;
};


} // namespace http
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
//#include "impl/connection.ipp"
#endif

#endif
