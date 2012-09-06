#define USE_SSL_BOOSTCONNECT

#include <iostream>
#include <boostconnect/connection_type/async_connection.hpp>
#include <boostconnect/connection_type/sync_connection.hpp>
#include <boostconnect/client.hpp>
#include <boostconnect/server.hpp>

#include <boost/format.hpp>

int main()
{  
  boost::asio::io_service io_service;
  boost::shared_ptr<boost::asio::streambuf> request_buf(new boost::asio::streambuf());
  {
    std::ostream os(request_buf.get());
    os << "GET /images/portal/logo-portal-top2.png HTTP/1.1\r\n";
    os << "Host: www.hatena.ne.jp\r\n";
    os << "Connection: close\r\n";
    os << "\r\n";
  }

  bstcon::client client(
    io_service,
    bstcon::connection_type::async
    );

  bstcon::client::connection_ptr connection = client(
    "www.hatena.ne.jp",
    request_buf,
    [](const bstcon::client::connection_ptr response_tmp,const boost::system::error_code&)->void{std::cout << "Connection End" << std::endl;}
  );

  io_service.run();

  std::cout << connection->get_response()->status_code << std::endl;
  std::cout << connection->get_response()->body;

  return 0;
}

//
//int main()
//{
//  typedef bstcon::request request_type;
//  typedef bstcon::response response_type;
//  //typedef boost::function<void(const request_type&,response_type&)> RequestHandler;
//
//  typedef boost::system::error_code error_code;
//  boost::asio::io_service io_service;
//
//  boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);
//
//  bstcon::server service(io_service,5600);
//  service.start(
//    [](const request_type& req,response_type& res)
//    {
//      res.status_code = 200;
//      res.http_version = "1.1";
//      res.status_message = "OK";
//      res.body = "<html><body><i><b>Ç›Ç°Å[Åô</b></i></body></html>";
//      res.header["Content-Length"] = (boost::format("%d") % res.body.size()).str();
//    }
//  );
//
//  try{
//    const std::string host = "127.0.0.1";
//    bstcon::client client(
//      io_service,
//      bstcon::connection_type::async
//      );
//
//    boost::shared_ptr<boost::asio::streambuf> buf1(new boost::asio::streambuf());
//    {
//     std::ostream os1(buf1.get());
//      os1 << "GET / HTTP/1.1\r\n";
//      os1 << "Host: "+host+"\r\n";
//      os1 << "Connection: close\r\n";
//      os1 << "\r\n";
//    }
//    const bstcon::client::connection_ptr connection1 = 
//      client(
//        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), 5600),
//        buf1,
//        [&host](const bstcon::client::connection_ptr connection,const error_code&)->void{std::cout << "THIS is Handler: "+host+"\n"+connection->get_response()->body + "\n\n";}
//      );
//    
//    boost::shared_ptr<boost::asio::streambuf> buf2(new boost::asio::streambuf());
//    {
//      std::ostream os2(buf2.get());
//      os2 << "GET / HTTP/1.1\r\n";
//      os2 << "Host: "+host+"\r\n";
//      os2 << "Connection: close\r\n";
//      os2 << "\r\n";
//    }
//    const bstcon::client::connection_ptr connection2 = 
//      client(
//        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), 5600),
//        buf2,
//        [&host](const bstcon::client::connection_ptr connection,const error_code&)->void{std::cout << "THIS is Handler: "+host+"\n"+connection->get_response()->body + "\n\n";}
//      );
//        
//    io_service.run();
//  }
//  catch(bstcon::system::exception& e){
//    std::cout << e.what() << std::endl;
//  }
//  
//  return 0;
//}
//
