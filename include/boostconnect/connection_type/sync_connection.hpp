//
// sync_connection.hpp
// ~~~~~~~~~~
//
// “¯Šú‚Ì‚½‚ß‚ÌƒNƒ‰ƒXŒQ
//

#ifndef BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION
#define BOOSTCONNECT_CONNECTTYPE_SYNC_CONNECTION

#include <memory>
#include <boost/asio.hpp>
#include "connection_base.hpp"

namespace bstcon{
namespace connection_type{

class sync_connection : public connection_common<sync_connection>{
public:
  sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
  {
    socket_ = socket;
  }
  virtual ~sync_connection(){}
  
  //’Ç‰Á
  connection_ptr operator() (
    const std::string& host,
    boost::shared_ptr<boost::asio::streambuf> buf,
    //error_code& ec,
    ReadHandler handler
    )
  {
    handler_ = handler;

    // Resolve Start
    boost::asio::ip::tcp::resolver resolver(socket_->get_io_service());
    boost::asio::ip::tcp::resolver::query query(host,socket_->service_protocol());
    boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query);

    // Connect Start
    boost::asio::connect(socket_->lowest_layer(),ep_iterator);
#ifdef USE_SSL_BOOSTCONNECT
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);    
#else
    socket_->handshake();    
#endif
    boost::asio::write(*socket_.get(),*buf.get());
    
    // Read Start
    reader_.reset(new reader());
    reader_->read_starter(*socket_.get(),boost::bind(&sync_connection::handle_read,this,boost::asio::placeholders::error));

    return this->shared_from_this();
  }

  connection_ptr operator() (
    const endpoint_type& ep,
    boost::shared_ptr<boost::asio::streambuf> buf,
    ReadHandler handler
    )
  {
    handler_ = handler;

    // Connect Start
    socket_->lowest_layer().connect(ep);
#ifdef USE_SSL_BOOSTCONNECT
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);
#else
    socket_->handshake();
#endif 
    boost::asio::write(*socket_.get(),*buf.get());
    
    // Read Start
    reader_.reset(new reader());
    reader_->read_starter(*socket_.get(),boost::bind(&sync_connection::handle_read,this,boost::asio::placeholders::error));

    return this->shared_from_this();
  }

private:
  void handle_read(const error_code& ec)
  {
    //std::cout << "SYNC";
    handler_(ec);
    //std::cout << "END";
  }
  
  ReadHandler handler_;
};

} // namespace connection_type
} // namespace bstcon

#endif
