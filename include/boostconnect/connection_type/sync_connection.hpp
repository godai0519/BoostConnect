//
// async_connection.hpp
// ~~~~~~~~~~
//
// 非同期のためのクラス群
//

#ifndef TWIT_LIB_PROTOCOL_CONNECTTYPE_SYNC_CONNECTION
#define TWIT_LIB_PROTOCOL_CONNECTTYPE_SYNC_CONNECTION

#include <memory>
#include <boost/asio.hpp>
#include "connection_base.hpp"
//#include "../application_layer/layer_base.hpp"

namespace oauth{
namespace protocol{
namespace connection_type{

class sync_connection : public connection_base{
public:
  sync_connection(const boost::shared_ptr<application_layer::socket_base>& socket)
  {
    socket_ = socket;
    busy = false;
  }
  virtual ~sync_connection(){}
  
  //追加
  response_type operator() (
    const std::string& host,
    boost::asio::streambuf& buf,
    //error_code& ec,
    ReadHandler handler
    )
  {
    common_init(handler);

    // Resolve Start
    boost::asio::ip::tcp::resolver resolver(socket_->get_io_service());
    boost::asio::ip::tcp::resolver::query query(host,socket_->service_protocol());
    boost::asio::ip::tcp::resolver::iterator ep_iterator = resolver.resolve(query);

    // Connect Start
    boost::asio::connect(socket_->lowest_layer(),ep_iterator);
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);    
    boost::asio::write(*socket_.get(),buf);
    
    // Read Start
    reader_.reset(new reader());
    reader_->read_starter(*socket_.get(),boost::bind(&sync_connection::handle_read,this,boost::asio::placeholders::error));
        
    busy = false;
    return reader_->get_response();
  }

  response_type operator() (
    const endpoint_type& ep,
    boost::asio::streambuf& buf,
    ReadHandler handler
    )
  {
    common_init(handler);

    // Connect Start
    socket_->lowest_layer().connect(ep);
    socket_->handshake(application_layer::socket_base::ssl_socket_type::client);    
    boost::asio::write(*socket_.get(),buf);
    
    // Read Start
    reader_.reset(new reader());
    reader_->read_starter(*socket_.get(),boost::bind(&sync_connection::handle_read,this,boost::asio::placeholders::error));

    busy = false;
    return reader_->get_response();
  }

private:
  void common_init(ReadHandler& handler)
  {
    if(busy)
    {
      oauth::system::throw_error(
        oauth::system::error_code(oauth::system::error::busy,oauth::system::client_category),
        "Sync"
        );
      //例外！！
    }
    busy = true;
    handler_ = handler;
  }

  void handle_read(const error_code& ec)
  {
    //std::cout << "SYNC";
    handler_(ec);
    //std::cout << "END";
  }
  
  ReadHandler handler_;
  bool busy;
};

} // namespace connection_type
} // namespace protocol
} // namespace oauth

#endif
