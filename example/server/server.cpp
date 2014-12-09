#define USE_SSL_BOOSTCONNECT
#include <iostream>
#include <boostconnect/server.hpp>
#include <boostconnect/session_type/http_session.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

int main()
{    
    typedef bstcon::server<bstcon::session::http_session> http_server;

    typedef bstcon::request request_type;
    typedef boost::shared_ptr<bstcon::session::http_session> session_ptr;

    // make server(http://127.0.0.1:5600/)
    auto const io_service = boost::make_shared<boost::asio::io_service>();
    http_server server(io_service, 5600);

    // start server
    server.start(
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
            
            auto response = boost::make_shared<bstcon::response>();
            response->http_version = "1.1";
            response->status_code = 200;
            response->reason_phrase = "OK";
            
#if 0
            response->body = "<html><body><i><b>Hello World</b></i></body></html>";
            response->header["Content-Length"] = std::to_string(response->body.length());
            session->set_all(response);
#endif

#if 1
            response->header["Transfer-Encoding"] = "chunked";
            session->set_headers(
                response,
                [](session_ptr session)
                {
                    session->set_chunk(
                        "<html><body><i><b>Hello World</b></i></body></html>", // chunk message
                        [](session_ptr session)
                        {
                            session->set_chunk(); // END chunk message
                        }
                    );
                }
            );
#endif

            return;
        }
    );

    io_service->run();

    return 0;
}
