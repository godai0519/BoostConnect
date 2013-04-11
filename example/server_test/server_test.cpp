#include <iostream>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boostconnect/config.hpp>
#include <boostconnect/server.hpp>
#include <boostconnect/client.hpp>

int main()
{
    typedef boost::system::error_code error_code;
    typedef bstcon::request request_type;
    typedef bstcon::client::response_type response_type; // = boost::shared_ptr<bstcon::response>
    typedef bstcon::client::connection_ptr connection_ptr;
    typedef boost::shared_ptr<bstcon::session::session_base> session_ptr;

    // io_service running
    boost::asio::io_service io_service;
    boost::scoped_ptr<boost::asio::io_service::work> worker(new boost::asio::io_service::work(io_service));
    std::thread th(boost::bind(&boost::asio::io_service::run, &io_service));

    // make server(http://127.0.0.1:5600/)
    bstcon::server service(io_service, 5600);

    // start server
    service.start(
        [&](const boost::shared_ptr<request_type> req, session_ptr session) -> void
        {
            //View Request
            std::string disp("--- Request Start ---\n");
            disp += "Method: " + req->method + "\n";
            disp += "Path: " + req->path + "\n";
            disp += "Version: " + req->http_version + "\n\n";
            BOOST_FOREACH(const bstcon::request::header_type::value_type& param, req->header)
            {
                disp += param.first + ": " + param.second + "\n";
            }
            disp += "\nBody: " + req->body + "\n";
            disp += "--- Request End ---\n\n";
            std::cout << disp << std::endl;

            std::map<std::string,std::string> param;
            param["Transfer-Encoding"] = "chunked";

            auto f1 = session->set_headers(200, "OK", "1.1", param);
            while(!f1.valid()) io_service.run_one();

            std::string body = "<html><body><i><b>Hello!</b></i></body></html>";

            auto f2 = session->set_chunk(body.size(), body);
            while(!f2.valid()) io_service.run_one();
            
            session->set_chunk(0);

            return;
        }
    );

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

    // Make: client using async 
    bstcon::client client(
        io_service,
        bstcon::connection_type::async // important!
        );

    // Challenge TCP connection establishment
    bstcon::client::connection_ptr connection = client(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 5600),
        [&](bstcon::client::connection_ptr connection_inner, boost::system::error_code ec)->void // Callback
        {
            assert(connection == connection_inner); // guarantee that

            if(ec)
            {
                // Bad connection
                throw;
            }

            // Nested lambda
            auto connection_ = connection;
            auto request_close_ = request_close;

            // Send First request
            connection->send(
                request_keep,
                [connection_, request_close_](response_type response, boost::system::error_code ec)->void // Callback
                {
                    if(ec && ec != boost::asio::error::eof)
                    {
                        // can't send request or read response
                        throw;
                    }

                    // Show first response
                    std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
                    std::cout << response->body + "\n\n" <<std::endl;

                    // Send Second request (Continue request)
                    connection_->send(
                        request_close_,
                        [](bstcon::client::response_type response, boost::system::error_code ec)->void  // Callback
                        {
                            if(ec && ec != boost::asio::error::eof) throw;

                            // Show second response
                            std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
                            std::cout << response->body + "\n\n" << std::endl;
                        }
                    );
                }
            );
        }
    );

    // join
    th.join();

    return 0;
}
