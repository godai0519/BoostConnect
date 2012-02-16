#include <iostream>
#include <boostconnect/application_layer/tcp_layer.hpp>
#include <boostconnect/application_layer/ssl_layer.hpp>
#include <boostconnect/connection_type/async_connection.hpp>
#include <boostconnect/connection_type/sync_connection.hpp>
#include <boostconnect/response_reader/response_container.hpp>
#include <boostconnect/client.hpp>

#include <boost/thread.hpp>

int main()
{
	boost::asio::io_service io_service;
	boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);

	oauth::protocol::client client(
		io_service,
		ctx,
		oauth::protocol::connection_type::async
		);

	std::string host = "www.google.co.jp";
	boost::system::error_code ec;
	boost::asio::streambuf buf;
	std::ostream os(&buf);
	{
		os << "GET / HTTP/1.1\r\n";
		os << "Host: "+host+"\r\n";
		os << "Connection: close\r\n";
		os << "\r\n";
	}

	client(host,buf,ec);
	io_service.run();
	auto response = client.get_response();
	
	return 0;
}
