#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <string>

struct verified_account
{
	std::string access_token;
	std::string access_secret;
};


int main()
{
    boost::property_tree::ptree pt;
    std::vector<verified_account> my_account_;
    
    boost::property_tree::read_xml("test.xml", pt);
    
	BOOST_FOREACH(const boost::property_tree::ptree::value_type& child, pt.get_child("account"))
	{
		boost::optional<std::string> token  = child.second.get_optional<std::string>("<xmlattr>.token" );
		boost::optional<std::string> secret = child.second.get_optional<std::string>("<xmlattr>.secret");
		if(token && secret)
		{
            verified_account tmp = {token.get(), secret.get()};
			my_account_.push_back(tmp);
		}
	}

    /*verified_account one = {"A","B"}, two = {"C","D"};
    my_account_.push_back(one);
    my_account_.push_back(two);
    
	BOOST_FOREACH(const verified_account& data, my_account_)
	{
		auto& child = pt.add("account.data", "");
		child.put("<xmlattr>.token",  data.access_token );
		child.put("<xmlattr>.secret", data.access_secret);
	}

	boost::property_tree::write_xml(
		"test.xml",
		pt,
		std::locale(),
		boost::property_tree::xml_parser::xml_writer_make_settings(' ', 2, boost::property_tree::xml_parser::widen<char>("utf-8"))
	);
    */
    return 0;
}


//#define USE_SSL_BOOSTCONNECT
//
//#include <iostream>
//#include <boostconnect/connection_type/async_connection.hpp>
//#include <boostconnect/connection_type/sync_connection.hpp>
//#include <boostconnect/client.hpp>
//#include <boostconnect/server.hpp>
//
//#include <boost/format.hpp>
//
//int main()
//{    
//    typedef bstcon::request request_type;
//    typedef bstcon::response response_type;
//    typedef boost::system::error_code error_code;
//
//    boost::asio::io_service io_service;
//    boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);
//
//    bstcon::server service(io_service,5600);
//    service.start(
//        [](const request_type& req,response_type& res)
//        {
//            res.status_code = 200;
//            res.http_version = "1.1";
//            res.status_message = "OK";
//            res.body = "<html><body><i><b>みぃー☆</b></i></body></html>";
//            res.header["Content-Length"] = (boost::format("%d") % res.body.size()).str();
//        }
//    );
//
//    boost::shared_ptr<boost::asio::streambuf> request_buf1(new boost::asio::streambuf());
//    {
//        std::ostream os(request_buf1.get());
//        os << "GET /images/portal/logo-portal-top2.png HTTP/1.1\r\n";
//        os << "Host: www.hatena.ne.jp\r\n";
//        os << "Connection: Keep-Alive\r\n";
//        os << "\r\n";
//    }
//    boost::shared_ptr<boost::asio::streambuf> request_buf2(new boost::asio::streambuf());
//    {
//        std::ostream os(request_buf2.get());
//        os << "GET /images/portal/logo-portal-top2.png HTTP/1.1\r\n";
//        os << "Host: www.hatena.ne.jp\r\n";
//        os << "Connection: close\r\n";
//        os << "\r\n";
//    }
//    //boost::shared_ptr<boost::asio::streambuf> request_local(new boost::asio::streambuf());
//    //{
//    //    std::ostream os(request_local.get());
//    //    os << "GET / HTTP/1.1\r\n";
//    //    os << "Host: 127.0.0.1\r\n";
//    //    os << "Connection: close\r\n";
//    //    os << "\r\n";
//    //}
//
//    bstcon::client client(
//        io_service,
//        bstcon::connection_type::async
//        //bstcon::connection_type::sync
//        );
//
//    //bstcon::client::connection_ptr connection = client("google.co.jp",[](bstcon::client::connection_ptr,error_code){});
//    //boost::shared_ptr<bstcon::response> response = connection->send(request_local);
//    //
//	//std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
//	//std::cout << response->body + "\n\n" <<std::endl;
//    
//
//    bstcon::client::connection_ptr connection = client(
//        (std::string)"www.hatena.ne.jp",
//        [&connection,request_buf1,request_buf2](bstcon::client::connection_ptr, boost::system::error_code ec)->void
//        {
//            auto connection_auto = connection;
//            auto request_buf2_auto = request_buf2;
//            connection->send(
//                request_buf1,
//                [connection_auto,request_buf2_auto](bstcon::client::response_type response, boost::system::error_code ec)
//                {
//                    if(!!ec) return;
//
//                    std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
//                    std::cout << response->body + "\n\n" <<std::endl;
//
//                    connection_auto->send(
//                        request_buf2_auto,
//                        [](bstcon::client::response_type response, boost::system::error_code ec)
//                        {
//                            if(!!ec) return;
//
//                            std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
//                            std::cout << response->body + "\n\n" << std::endl;
//                        }
//                    );
//                }
//            );
//        }
//    );
//
//    //bstcon::client::connection_ptr connection_local = client(
//    //    boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 5600),
//    //    [&connection_local,request_local](bstcon::client::connection_ptr, boost::system::error_code ec)->void
//    //    {
//    //        connection_local->send(
//    //            request_local,
//    //            [](bstcon::client::response_type response, boost::system::error_code ec)
//    //            {
//    //                if(!!ec) return;
//
//    //                std::cout << "Status Code: " << response->status_code << " " << response->status_message << std::endl;
//    //                std::cout << response->body + "\n\n" <<std::endl;
//    //            }
//    //        );
//    //    }
//    //);
//
//    io_service.run();
//
//    return 0;
//}
//
