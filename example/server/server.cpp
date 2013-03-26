#include <iostream>
#include <boostconnect/server.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

int main()
{    
    typedef bstcon::request request_type;
    typedef bstcon::response response_type;

    // make server(http://127.0.0.1:5600/)
    boost::asio::io_service io_service;
    bstcon::server service(io_service,  5600);

    // start server
    service.start(
        [](const request_type& req,response_type& res) -> void
        {
            //View Request
            std::string disp("--- Request Start ---\n");
            disp += "Method: " + req.method + "\n";
            disp += "Path: " + req.path + "\n";
            disp += "Version: " + req.http_version + "\n\n";
            BOOST_FOREACH(const bstcon::request::header_type::value_type& param, req.header)
            {
                disp += param.first + ": " + param.second + "\n";
            }
            disp += "\nBody: " + req.body + "\n";
            disp += "--- Request End ---\n\n";
            std::cout << disp << std::endl;

            //Make Response
            res.status_code = 200;
            res.http_version = "1.1";
            res.status_message = "OK";
            res.body = "<html><body><i><b>Hello!</b></i></body></html>";
            res.header["Content-Length"] = (boost::format("%d") % res.body.size()).str();

            return;
        }
    );

    io_service.run();

    return 0;
}
