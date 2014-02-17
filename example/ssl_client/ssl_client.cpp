#define USE_SSL_BOOSTCONNECT
#include <boostconnect/client.hpp>
#include <boostconnect/connection_type/async_connection.hpp>
#include <boostconnect/application_layer/ssl_socket.hpp>
#include <boostconnect/protocol_type/http.hpp>

int main()
{
    bstcon::request request;
    request.method = "GET";
    request.path = "/";
    request.http_version = "1.1";
    request.header["Host"] = "www.google.co.jp";
    request.header["Connection"] = "Keep-Alive";
    request.body = "";
    
    // ~~~ Difference is Just this! ~~~
    typedef bstcon::client<bstcon::connection_type::async_connection, bstcon::protocol_type::http> client_type;
    auto io_service = boost::make_shared<boost::asio::io_service>();
    auto client = boost::make_shared<client_type>(io_service); // Use context ref
    // ~~~ Difference end ~~~

	boost::asio::ssl::context ctx(*io_service, boost::asio::ssl::context_base::sslv3_client); //SSL3
	(*client)(
		"www.google.co.jp",
		ctx,
		[&request](boost::shared_ptr<bstcon::protocol_type::http> const& service, boost::system::error_code const& ec)
		{
			service->request(
				request,
				[&request, service](boost::shared_ptr<bstcon::response> const& response1)
				{
					std::cout << response1->body << std::endl;

					request.header["Connection"] = "close";
					service->request(
						request,
						[](boost::shared_ptr<bstcon::response> const& response2)
						{
							std::cout << response2->body << std::endl;
							return;
						});

					return;
				});

			return;
		});

    io_service->run();
    
    return 0;
}

