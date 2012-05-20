//
// async_connection.hpp
// ~~~~~~~~~~
//
// ”ñ“¯Šú‚Ì‚½‚ß‚ÌƒNƒ‰ƒXŒQ
//

#ifndef BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION
#define BOOSTCONNECT_CONNECTTYPE_ASYNC_CONNECTION

#include <memory>
#include <boost/asio.hpp>
#include "connection_base.hpp"
#include "../system/error_code.hpp"

namespace bstcon{
namespace connection_type{

class async_connection : public connection_common<async_connection>{
public:
  async_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
  {
    socket_ = socket;
    resolver_ = new boost::asio::ip::tcp::resolver(socket_->get_io_service());
    reader_.reset(new reader());
  }
  virtual ~async_connection(){}

  void close()
  {
    socket_->close();
    return;
  }
  
  //’Ç‰Á  
  connection_ptr operator() (
    const std::string& host,
    boost::shared_ptr<boost::asio::streambuf> buf,
    ReadHandler handler
    )
  {
    buf_ = buf;
    handler_ = handler;    

    boost::asio::ip::tcp::resolver::query query(host,socket_->service_protocol());
    resolver_->async_resolve(query,
      boost::bind(&async_connection::handle_resolve,this,
      boost::asio::placeholders::iterator,
      boost::asio::placeholders::error)); //handle_resolve‚Ö

    return this->shared_from_this();
  }

  connection_ptr operator() (
    const endpoint_type& ep,
    boost::shared_ptr<boost::asio::streambuf> buf,
    ReadHandler handler
    )
  {
    buf_ = buf;
    handler_ = handler;    
    
    socket_->lowest_layer().async_connect(
      ep,

      boost::bind(&async_connection::handle_connect,this,
        boost::asio::placeholders::error));

    return this->shared_from_this();
  }

private:
  ReadHandler handler_;

  boost::asio::ip::tcp::resolver* resolver_;
  boost::shared_ptr<boost::asio::streambuf> buf_;
  void handle_resolve(boost::asio::ip::tcp::resolver::iterator ep_iterator,const boost::system::error_code& ec)
  {
    if(!ec)
    {
      boost::asio::async_connect(socket_->lowest_layer(),ep_iterator,
        boost::bind(&async_connection::handle_connect,this,
          boost::asio::placeholders::error));
    }
    else std::cout << "Error Resolve!?\n" << ec.message() << std::endl;
  }
  void handle_connect(const boost::system::error_code& ec)
  {
    if(!ec)
    {
#ifdef USE_SSL_BOOSTCONNECT
      socket_->async_handshake(application_layer::socket_base::ssl_socket_type::client,
        boost::bind(&async_connection::handle_handshake,this,
          boost::asio::placeholders::error));
#else
      socket_->async_handshake(
        boost::bind(&async_connection::handle_handshake,this,
          boost::asio::placeholders::error));
#endif
    }
    else std::cout << "Error Connect!?" << std::endl;
  }
  void handle_handshake(const boost::system::error_code& ec)
  {
    if(!ec)
    {
      boost::asio::async_write(*socket_.get(),*buf_.get(),
        boost::bind(&async_connection::handle_write,this,
          boost::asio::placeholders::error));
    }
    else std::cout << "Error HandShake!?\n" << ec.message() << std::endl;
  }
  void handle_write(const boost::system::error_code& ec)
  {
    if(!ec)
    {
      reader_->async_read_starter(*socket_.get(),
        boost::bind(&async_connection::handle_read,this,
          boost::asio::placeholders::error));
    }
    else std::cout << "Error Write!?" << std::endl;
  }
  void handle_read(const error_code& ec)
  {
    //std::cout << "ASYNC";
    handler_(ec);
    //std::cout << "END";
  }
};

} // namespace connection_type
} // namespace bstcon

#endif
