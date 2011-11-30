#include <iostream>
#include <boostconnect/application_layer/ssl_layer.hpp>
#include <boostconnect/application_layer/tcp_layer.hpp>
#include <boostconnect/application_layer/connector.hpp>
#include <boostconnect/client.hpp>

#define ASYNC

int main()
{
	connector::io_service io_service;
	
#ifdef ASYNC

	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query("google.com","https");
	connector::endpoint ep(*resolver.resolve(query));	
	boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);
	
	boost::asio::streambuf buf;
	std::ostream os(&buf);
	os << "GET / HTTP/1.1\r\nConnection: close\r\n\r\n";

	client cl(io_service,ctx,ep);

	cl.sync_write(buf);
	cl.sync_read(buf);

	std::cout << boost::asio::buffer_cast<const char*>(buf.data());

	cl.close();
#endif

#ifdef SSL_SYNC

	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query("google.com","https");
	connector::endpoint ep(*resolver.resolve(query));	
	boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);
	
	boost::asio::streambuf buf;
	std::ostream os(&buf);
	os << "GET / HTTP/1.1\r\nConnection: close\r\n\r\n";

	client cl(io_service,ctx,ep);

	cl.sync_write(buf);
	cl.sync_read(buf);

	std::cout << boost::asio::buffer_cast<const char*>(buf.data());

	cl.close();
#endif

	return 0;
}
