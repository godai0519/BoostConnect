#include <iostream>
#include <boostconnect/config.hpp>
#include <boostconnect/server.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

int main()
{    
    typedef bstcon::request request_type;
    typedef boost::shared_ptr<bstcon::session::session_base> session_ptr;

    // make server(http://127.0.0.1:5600/)
    boost::asio::io_service io_service;
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
            
            std::string body = "<html><body><i><b>Hello!</b></i></body></html>";
            std::map<std::string,std::string> param;

            /*{
                param["Content-Length"] = std::to_string(body.length());

                auto f1 = session->set_headers(200, "OK", "1.1", param);
                while(!f1.valid()) io_service.run_one();

                auto f2 = session->set_body(body);
                while(!f2.valid()) io_service.run_one();
            }*/
            
            {
                param["Transfer-Encoding"] = "chunked";
                //param["Connection"] = "Keep-Alive";

                auto f1 = session->set_headers(200, "OK", "1.1", param);
                //while(!f1.valid()) io_service.run_one();


                auto f2 = session->set_chunk(body.size(), body);
                //while(!f2.valid()) io_service.run_one();
            
                session->set_chunk(0);
            }

            return;
        }
    );

    io_service.run();

    return 0;
}
