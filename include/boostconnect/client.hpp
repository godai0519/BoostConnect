//
// client.hpp
// ~~~~~~~~~~
//
// クライアント接続のメイン管理クラス
//

#ifndef BOOSTCONNECT_CLIENT
#define BOOSTCONNECT_CLIENT

#include <memory>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include "manager.hpp"
#include "application_layer/socket_base.hpp"
#include "application_layer/tcp_socket.hpp"
#include "application_layer/ssl_socket.hpp"
#include "connection_type/connection_base.hpp"
#include "connection_type/async_connection.hpp"
#include "connection_type/sync_connection.hpp"

namespace bstcon{

//複数の通信を同時に要求した際の保証はしない
class client : boost::noncopyable{
public:
  typedef boost::asio::io_service io_service;
  typedef boost::system::error_code error_code;
  typedef boost::shared_ptr<bstcon::response> response_type;

  typedef boost::shared_ptr<bstcon::application_layer::socket_base>   socket_ptr;
  typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;

  typedef boost::function<void (const boost::shared_ptr<bstcon::response>,const error_code&)> ClientHandler;
  typedef boost::function<void (const boost::shared_ptr<bstcon::response>,const error_code&)> EveryChunkHandler;

  //// TODO: C++11にて可変長引数に対応させる
  //template<class ...Args>
  //client(boost::asio::io_service &io_service,boost::asio::ip::tcp::endpoint& ep,Args... args)
  //{
  //  connector_.reset(new connector(io_service,ep,arg...));
  //  socket_layer_ = connector_->get_layer();
  //}

  //コンストラクタの引数でconnection_type_初期化しなくては。
  //現在のasync,sync判断は美しくない！

  //TCP
  client(io_service &io_service,const connection_type::connection_type& connection_type=connection_type::sync)
    : connection_type_(connection_type), io_service_(io_service)
#ifdef USE_SSL_BOOSTCONNECT
    , ctx_(nullptr)
#endif
  {}
  
#ifdef USE_SSL_BOOSTCONNECT
  //SSL
  typedef boost::asio::ssl::context context;
  client(io_service &io_service,context &ctx,const connection_type::connection_type& connection_type=connection_type::sync) : io_service_(io_service), connection_type_(connection_type), ctx_(&ctx){}
#endif
  
  // Use host
  const response_type operator() (
    const std::string& host,
    boost::shared_ptr<boost::asio::streambuf> buf,
    ClientHandler handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{},
    EveryChunkHandler chunk_handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{}
    )
  {
    connection_ptr connection = crerate_connection();
    manager_.run(connection);
    
    connection->operator()(host,buf,
      boost::bind(&client::handler,this,_1,connection,handler),
      chunk_handler
      );
    return connection->get_response();
  }
  const response_type operator() (
    const std::string& host,
    boost::shared_ptr<boost::asio::streambuf> buf,
    error_code& ec,
    ClientHandler handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{},
    EveryChunkHandler chunk_handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{}
    )
  {
    try
    {
      return (*this)(host,buf,handler,chunk_handler);
    }
    catch(const boost::system::system_error &e)
    {
      ec = e.code(); //例外からerror_codeを抜き取る
      return response_type(); //レスポンスが空のままというのもアレなので，作成済みのレスポンスのアドレスを取得
    }
  }

  // Use EndPoint
  const response_type operator() (
    const boost::asio::ip::tcp::endpoint& host,
    boost::shared_ptr<boost::asio::streambuf> buf,
    ClientHandler handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{},
    EveryChunkHandler chunk_handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{}
    )
  {
    connection_ptr connection = crerate_connection();
    manager_.run(connection);

    connection->operator()(host,buf,
      boost::bind(&client::handler,this,_1,connection,handler),
      chunk_handler
      );
    return connection->get_response();
  }
  const response_type operator() (
    const boost::asio::ip::tcp::endpoint& host,
    boost::shared_ptr<boost::asio::streambuf> buf,
    error_code& ec,
    ClientHandler handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{},
    EveryChunkHandler chunk_handler = [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{}
    )
  {
    try
    {
      return (*this)(host,buf,handler,chunk_handler);
    }
    catch(const boost::system::system_error &e)
    {
      ec = e.code(); //例外からerror_codeを抜き取る
      return response_type(); //レスポンスが空のままというのもアレなので，作成済みのレスポンスのアドレスを取得
    }
  }

  const std::string service_protocol() const
  {
#ifdef USE_SSL_BOOSTCONNECT
  return (ctx_==nullptr) ? "http" : "https";
#endif
  return "http";
  }

protected:
  inline socket_ptr create_socket()
  {
    socket_ptr socket;

#ifdef USE_SSL_BOOSTCONNECT
  if(ctx_ != nullptr)
      socket.reset(new bstcon::application_layer::ssl_socket(io_service_,*ctx_));
  else
#endif
      socket.reset(new bstcon::application_layer::tcp_socket(io_service_));

    return socket;
  }
  inline const connection_ptr crerate_connection()
  {
    connection_ptr connection;

    if(connection_type_ == connection_type::sync)
      connection.reset(new bstcon::connection_type::sync_connection(create_socket()));
    else 
      connection.reset(new bstcon::connection_type::async_connection(create_socket()));

    return connection;
  }

  void handler(const error_code& ec,connection_ptr connection,ClientHandler h) const
  {
    h(connection->get_response(),ec);
    manager_.stop(connection);
    return;
  }

private:
#ifdef USE_SSL_BOOSTCONNECT
  context *ctx_;
#endif
  boost::asio::io_service& io_service_;
  connection_type::connection_type connection_type_;

  mutable bstcon::manager<connection_type::connection_base> manager_;
};

} // namespace bstcon

#endif
