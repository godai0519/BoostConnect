//
// http_session.hpp
// ~~~~~~~~~~
//
// HTTP通信のセッションを管理
//

#ifndef BOOSTCONNECT_SESSION_HTTP
#define BOOSTCONNECT_SESSION_HTTP

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include "session_base.hpp"
#include "../application_layer/socket_base.hpp"

namespace bstcon{
namespace session{
  
class http_session : public session_common<http_session> {
public:
  typedef boost::asio::io_service io_service;
  typedef boost::asio::ssl::context context;
  typedef boost::system::error_code error_code;

  bstcon::application_layer::socket_base::lowest_layer_type& lowest_layer()
  {
    return socket_->lowest_layer();
  }
  
  http_session(io_service& io_service): socket_busy_(false),read_timer_(io_service)
  {
    socket_ = new bstcon::application_layer::tcp_socket(io_service);
  }
  http_session(io_service& io_service,context& ctx): socket_busy_(false),read_timer_(io_service)
  {
    socket_ = new bstcon::application_layer::ssl_socket(io_service,ctx);
  }
  virtual ~http_session()
  {
    delete socket_;
  }

  void start(RequestHandler handler,CloseHandler c_handler)
  {
    if(socket_busy_) return; //例外予定
    socket_busy_ = true;
    handler_ = handler;
    c_handler_ = c_handler;

    socket_->async_handshake(boost::asio::ssl::stream_base::server,
      boost::bind(&http_session::handle_handshake,shared_from_this(),
        boost::asio::placeholders::error));
  }
  void end(CloseHandler c_handler)
  {
    socket_->close();

    c_handler(static_cast<boost::shared_ptr<session_base>>(shared_from_this()));
    //delete this;
    return;
  }

private:
  void handle_handshake(const error_code& ec)
  {
    if(!ec)
    {
      read_buf_.reset(new boost::asio::streambuf());
      boost::asio::async_read_until(*socket_,*read_buf_.get(),
        "\r\n\r\n",
        boost::bind(&http_session::handle_header_read,shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

      read_timer_.expires_from_now(boost::posix_time::seconds(5));
      read_timer_.async_wait(
        boost::bind(&http_session::read_timeout,shared_from_this(),boost::asio::placeholders::error));
    }
    //else delete this; //例外
  }

  void read_timeout(const error_code& ec)
  {
    //socket_->shutdown(boost::asio::socket_base::shutdown_both);
    //socket_->close();
    //socket_->get_io_service().stop();
    //delete this;
    this->end(c_handler_);
    return;
  }

  void handle_header_read(const error_code& ec,std::size_t)
  {
    /*if(ec)
    {
      delete this;
      return;
    }*/

    if(!ec)
    {
      std::string request_str(boost::asio::buffer_cast<const char*>(read_buf_->data()));

      //リクエストヘッダーのパース
      request_str.erase(0,request_header_parser(request_str,request_));
      request_.body.append(request_str);
    
      size_t byte = int_parser(MAP_FIND_RETURN_OR_DEFAULT(request_.header,"Content-Length","0"));
      if( byte != 0 )
      {
        //長さがある
        read_buf_.reset(new boost::asio::streambuf());

        boost::asio::async_read(
          *socket_,
          *read_buf_.get(),
          boost::asio::transfer_at_least(byte - request_.body.length()),
          boost::bind(&http_session::handle_body_read,shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
      }
      else
      {
        //長さがない -> 長さが分からないならこの先読み込むのは危ない
        handle_request_read_complete();
      }
    }
    //else delete this; //例外
  }

  void handle_body_read(const error_code& ec,std::size_t sz)
  {
    if(!ec)
    {
      std::string body_str(boost::asio::buffer_cast<const char*>(read_buf_->data()));
      request_.body.append(body_str);

      handle_request_read_complete();
    }
    //else delete this; //例外
  }

  void handle_request_read_complete()
  {
    keep_alive_ = 
      MAP_FIND_RETURN_OR_DEFAULT(request_.header,"Connection","close") == "Keep-Alive" ||
      MAP_FIND_RETURN_OR_DEFAULT(request_.header,"Proxy-Connection","close") == "Keep-Alive";

    response_type response;
    handler_(request_,response);
    write_buf_.reset(new boost::asio::streambuf());

    //std::ostream os(write_buf_.get()); //TODO:

    if(response_generater(std::ostreambuf_iterator<char>(write_buf_.get()),response))
    {
      boost::asio::async_write(*socket_,*write_buf_.get(),
        boost::bind(&http_session::handle_write,shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
    //else delete this; //パース失敗
  }

  void handle_write(const error_code& ec,std::size_t sz)
  {
    if(!ec)
    {
      if(keep_alive_)
      {
        handle_handshake(error_code());
      }
      else
      {
        this->end(c_handler_);
        return;
      }
    }
    //else delete this; //例外
  }


  template<class OutputIterator>
  bool response_generater(OutputIterator& sink,const response_type& response) const
  {
    namespace karma = boost::spirit::karma;
    
    bool r = karma::generate(sink,
      "HTTP/" << karma::string << ' ' << karma::int_ << ' ' << karma::string << "\r\n",
      response.http_version,
      response.status_code,
      response.status_message);

    if(!response.header.empty())
    {
      r = r && karma::generate(sink,
        +(karma::string << ": " << karma::string << "\r\n"),
        response.header);
    }

    if(!response.body.empty())
    {
      r = r && karma::generate(sink,
        "\r\n" << karma::string,
        response.body);
    }

    return r;
  }

  const int int_parser(const std::string& base) const
  {
    namespace qi = boost::spirit::qi;

    int parsed = 0;
    qi::parse(base.cbegin(),base.cend(),qi::int_,parsed);

    return parsed;
  }

  const int request_header_parser(const std::string& request_str,bstcon::request& request_containar) const
  {
    namespace qi = boost::spirit::qi;

    auto line = +(qi::char_-' ') >> ' ' >> +(qi::char_-' ') >> ' ' >> qi::lit("HTTP/") >> +(qi::char_ - qi::lit("\r\n")) >> qi::lit("\r\n");
    auto field = +(qi::char_-':') >> qi::lit(": ") >> +(qi::char_-qi::lit("\r\n")) >> qi::lit("\r\n");
    auto rule =  line >> *field >> qi::lit("\r\n");

    auto it = request_str.cbegin();
    qi::parse(it,request_str.cend(),rule,
      request_containar.method,
      request_containar.uri,
      request_containar.http_version,
      request_containar.header);

    return std::distance(request_str.cbegin(),it);
  }

  request_type request_;

  std::unique_ptr<boost::asio::streambuf> read_buf_;
  std::unique_ptr<boost::asio::streambuf> write_buf_;

  bstcon::application_layer::socket_base *socket_;
  RequestHandler handler_;
  CloseHandler c_handler_;
  boost::asio::deadline_timer read_timer_;
  bool socket_busy_;
  bool keep_alive_;
};

} // namespace session
} // namespace bstcon

#endif
