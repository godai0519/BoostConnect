//
// client.hpp
// ~~~~~~~~~~
//
// クライアント接続のメイン管理クラス
//

#ifndef BOOSTCONNECT_CLIENT_HPP
#define BOOSTCONNECT_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

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
    client(io_service &io_service,const connection_type::connection_type& connection_type=connection_type::sync);
    
#ifdef USE_SSL_BOOSTCONNECT
    //SSL
    typedef boost::asio::ssl::context context;
    client(io_service &io_service,context &ctx,const connection_type::connection_type& connection_type=connection_type::sync);
#endif
    
    template<typename T>
    connection_ptr operator() (
        const T& host,
        ConnectionHandler handler
        );

    const std::string service_protocol() const;

protected:
    inline socket_ptr create_socket();
    inline const connection_ptr crerate_connection();

private:
#ifdef USE_SSL_BOOSTCONNECT
    context *ctx_;
#endif
    boost::asio::io_service& io_service_;
    connection_type::connection_type connection_type_;
};

} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "impl/client.ipp"
#endif

#endif
