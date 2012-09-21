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
    typedef boost::asio::ip::tcp::endpoint endpoint_type;
    typedef boost::shared_ptr<bstcon::response> response_type;

    typedef boost::shared_ptr<bstcon::application_layer::socket_base>   socket_ptr;
    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;
    typedef bstcon::connection_type::connection_base::ConnectionHandler ConnectionHandler;


    //// TODO: C++11にて可変長引数に対応させる
    //template<class ...Args>
    //client(boost::asio::io_service &io_service,boost::asio::ip::tcp::endpoint& ep,Args... args)
    //{
    //    connector_.reset(new connector(io_service,ep,arg...));
    //    socket_layer_ = connector_->get_layer();
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
    
    template<typename T>
    connection_ptr operator() (
        const T& host,
        ConnectionHandler handler
        )
    {
        auto connection = crerate_connection();
        connection->connect(host, handler);

        return connection;
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

private:
#ifdef USE_SSL_BOOSTCONNECT
    context *ctx_;
#endif
    boost::asio::io_service& io_service_;
    connection_type::connection_type connection_type_;
};

} // namespace bstcon

#endif
