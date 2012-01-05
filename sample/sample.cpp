#include <twit-library/protocol.hpp>
#include <map>
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>

int main()
{
	//ただの準備です
	boost::asio::io_service io_service;
	boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client); //SSL用
	boost::system::error_code ec;
	const std::string host("www.hatena.ne.jp");
	boost::asio::streambuf buf;
	std::ostream os(&buf);
	os << "GET / HTTP/1.1\r\nHost: " << host << "\r\nConnection: close\r\n\r\n"; //ただのリクエスト
	
	////
	//// 同期・HTTP通信
	////
	//{
	//	oauth::protocol::client client(io_service/*,oauth::protocol::connection_type::sync*/);
	//	client(host,buf,ec);
	//	
	//	//終了
	//}

	////
	//// 同期・SSL通信
	////
	//{
	//	oauth::protocol::client client(io_service,ctx/*,oauth::protocol::connection_type::sync*/);
	//	client(host,buf,ec);

	//	//終了
	//}
	
	////
	//// 非同期・HTTP通信
	////
	//{
	//	oauth::protocol::client client(io_service,oauth::protocol::connection_type::async);
	//	client(host,buf,ec);

	//	//何か

	//	io_service.run(); //通信を走らせる

	//	//終了
	//	return 0;
	//}

	//
	// 非同期・SSL通信
	//
	{
		oauth::protocol::client client(io_service,ctx,oauth::protocol::connection_type::async);
		client(host,buf,ec);

		//何か

		io_service.run(); //通信を走らせる

		//終了
	}
	
	return 0;
}
