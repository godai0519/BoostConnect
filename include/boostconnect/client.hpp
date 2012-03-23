//
// client.hpp
// ~~~~~~~~~~
//
// 接続のメイン管理クラス
//

#ifndef TWIT_LIB_PROTOCOL_CLIENT
#define TWIT_LIB_PROTOCOL_CLIENT

#include <memory>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include "application_layer/layer_base.hpp"
#include "application_layer/tcp_layer.hpp"
#include "application_layer/ssl_layer.hpp"
#include "connection_type/connection_base.hpp"
#include "connection_type/async_connection.hpp"
#include "connection_type/sync_connection.hpp"

namespace oauth{
namespace protocol{

//複数の通信を同時に要求した際の保証はしない
class client : boost::noncopyable{
public:
	typedef boost::asio::io_service io_service;
	typedef boost::system::error_code error_code;
	typedef std::shared_ptr<oauth::protocol::response> response_type;
#ifdef TWIT_LIB_PROTOCOL_APPLAYER_SSL_LAYER
	typedef boost::asio::ssl::context context;
#endif

	//// TODO: C++11にて可変長引数に対応させる
	//template<class ...Args>
	//client(boost::asio::io_service &io_service,boost::asio::ip::tcp::endpoint& ep,Args... args)
	//{
	//	connector_.reset(new connector(io_service,ep,arg...));
	//	socket_layer_ = connector_->get_layer();
	//}

	//コンストラクタの引数でconnection_type_初期化しなくては。
	//現在のasync,sync判断は美しくない！

	//TCP
	client(io_service &io_service,const connection_type::connection_type& ct=connection_type::sync) : socket_layer_(new application_layer::tcp_layer(io_service))
	{
		if(ct == connection_type::sync)
		{
			connection_type_.reset(new connection_type::sync_connection(socket_layer_));
		}
		else if(ct == connection_type::async)
		{
			connection_type_.reset(new connection_type::async_connection(socket_layer_));
		}
	}
	
#ifdef TWIT_LIB_PROTOCOL_APPLAYER_SSL_LAYER
	//SSL
	client(io_service &io_service,context &ctx,const connection_type::connection_type& ct=connection_type::sync) : ctx_(&ctx),socket_layer_(new application_layer::ssl_layer(io_service,ctx))
	{
		if(ct == connection_type::sync)
		{
			connection_type_.reset(new connection_type::sync_connection(socket_layer_));
		}
		else if(ct == connection_type::async)
		{
			connection_type_.reset(new connection_type::async_connection(socket_layer_));
		}
	}
#endif
	
	const response_type& operator() (
		const std::string& host,
		boost::asio::streambuf& buf,
	//	error_code& ec,
		application_layer::layer_base::ReadHandler handler = [](const error_code&)->void{}
		)
	{
		const response_type& response = socket_layer_->reset_response();
		connection_type_->operator()(host,buf,/*ec,*/handler);
		return response;
	}
	const response_type& operator() (
		const std::string& host,
		boost::asio::streambuf& buf,
		error_code& ec,
		application_layer::layer_base::ReadHandler handler = [](const error_code&)->void{}
		)
	{
		try
		{
			const response_type& response = (*this)(host,buf,handler);
			return response;
		}
		catch(const boost::system::system_error &e)
		{
			ec = e.code(); //例外からerror_codeを抜き取る
			const response_type& response = socket_layer_->get_response(); //レスポンスが空のままというのもアレなので，作成済みのレスポンスのアドレスを取得
			return response;
		}
	}

	void close()
	{
		socket_layer_->close();
		return;
	}

	const std::string service_protocol() const {return socket_layer_->service_protocol();}

	//response Service
	inline const response_type& get_response(){return socket_layer_->get_response();}
	//void reset_response(){socket_layer_->reset_response();}

private:
	std::shared_ptr<application_layer::layer_base> socket_layer_;
	std::shared_ptr<connection_type::connection_base> connection_type_;
#ifndef NO_SSL
	context *ctx_;
#endif
};

} // namespace protocol
} // namespace oauth

#endif
