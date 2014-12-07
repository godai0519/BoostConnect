#include <iostream>
#include <boostconnect/client.hpp>
#include <boostconnect/connection_type/sync_connection.hpp>
#include <boostconnect/application_layer/tcp_socket.hpp>
#include <boostconnect/protocol_type/http.hpp>

int main()
{
    typedef bstcon::client<bstcon::connection_type::sync_connection, bstcon::protocol_type::http> client_type;
    
    bstcon::request request;
    request.method = "GET";
    request.path = "/";
    request.http_version = "1.1";
    request.header["Host"] = "www.google.co.jp";
    request.header["Connection"] = "Keep-Alive";
    request.body = "";
    
    auto io_service = boost::make_shared<boost::asio::io_service>();
    auto client = boost::make_shared<client_type>(io_service);
    boost::shared_ptr<bstcon::protocol_type::http> connection = (*client)("www.google.co.jp").get();

    auto future1 = connection->request(request);
    auto response1 = future1.get();

    request.header["Connection"] = "close";
    
    auto future2 = connection->request(request);
    auto response2 = future2.get();

    std::cout << response1.body << std::endl;
    std::cout << response2.body << std::endl;
    
    return 0;
}
