#define USE_SSL_BOOSTCONNECT
#include <iostream>
#include <boostconnect/connection_type/async_connection.hpp>
#include <boostconnect/client.hpp>

int main()
{
    typedef boost::system::error_code error_code;
    typedef bstcon::client::response_type response_type; // = boost::shared_ptr<bstcon::response>
    typedef bstcon::client::connection_ptr connection_ptr;
    
    // Make: request header and body
    boost::shared_ptr<boost::asio::streambuf> request_keep(new boost::asio::streambuf());
    {
        std::ostream os(request_keep.get());
        os << "GET / HTTP/1.1\r\n";
        os << "Host: www.google.co.jp\r\n";
        os << "Connection: Keep-Alive\r\n";
        os << "\r\n";
    }
    boost::shared_ptr<boost::asio::streambuf> request_close(new boost::asio::streambuf());
    {
        std::ostream os(request_close.get());
        os << "GET / HTTP/1.1\r\n";
        os << "Host: www.google.co.jp\r\n";
        os << "Connection: close\r\n";
        os << "\r\n";
    }

    // Make: client using sync and SSL
    // ~~~ Difference is Just this! ~~~
    boost::asio::io_service io_service;
    boost::asio::ssl::context ctx(io_service, boost::asio::ssl::context_base::sslv3_client);
    bstcon::client client(
        io_service,
        ctx,
        bstcon::connection_type::sync // important!
        );
    // ~~~ Difference end ~~~
    
    // Challenge SSL connection establishment
    connection_ptr connection = client(
        std::string("www.google.co.jp"),
        [](bstcon::client::connection_ptr connection_inner, boost::system::error_code ec)->void
        {
            if(ec) throw;  // Bad connection
        }
    );

    {
        // Send First request
        std::future<response_type> f = connection->send(request_keep);
        const response_type response = f.get();

        // Show first response
        std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
        std::cout << response->body + "\n\n" <<std::endl;
    }

    //
    // --- Connection Keeping ---
    //

    {
        // Send Second request (Continue request)
        std::future<response_type> f = connection->send(request_close);
        const response_type response = f.get();
                    
        // Show second response
        std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
        std::cout << response->body + "\n\n" << std::endl;
    }
    
    return 0;
}
