#include <iostream>
#include <boostconnect/connection_type/async_connection.hpp>
#include <boostconnect/connection_type/sync_connection.hpp>
#include <boostconnect/client.hpp>
#include <boostconnect/server.hpp>

#include <boostconnect/session/manager.hpp>

#include <boost/format.hpp>

int main()
{
  typedef bstcon::request request_type;
  typedef bstcon::response response_type;
  typedef boost::function<void(const request_type&,response_type&)> RequestHandler;

  typedef boost::system::error_code error_code;
  boost::asio::io_service io_service;

  boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);

  bstcon::server service(io_service,5600);
  service.start(
    [](const request_type& req,response_type& res)
    {
      res.status_code = 200;
      res.http_version = "1.1";
      res.status_message = "OK";
      res.body = "<html><body><i><b>Ç›Ç°Å[Åô</b></i></body></html>";
      res.header["Content-Length"] = (boost::format("%d") % res.body.size()).str();
    }
  );

  try{  
    bstcon::client client(
      io_service,
      bstcon::connection_type::async
      );
  
    std::string host = "127.0.0.1";
    boost::system::error_code ec;
    boost::asio::streambuf buf;
    std::ostream os(&buf);
    {
      os << "GET / HTTP/1.1\r\n";
      os << "Host: "+host+"\r\n";
      os << "Connection: close\r\n";
      os << "\r\n";
    }
    const boost::shared_ptr<bstcon::response> response = 
      client(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 5600),
        buf,
        [&host,&response](const error_code&)->void{std::cout << "THIS is Handler: "+host+"\n"+response->body << std::endl;}
      );
    
    io_service.run();

  ////client.get_response();
  //
  //std::string host2 = "www.hatena.ne.jp";
  //boost::system::error_code ec2;
  //boost::asio::streambuf buf2;
  //std::ostream os2(&buf2);
  //{
  //  os2 << "GET / HTTP/1.1\r\n";
  //  os2 << "Host: "+host2+"\r\n";
  //  os2 << "Connection: close\r\n";
  //  os2 << "\r\n";
  //}
  //auto response2 = client(host2,buf2,/*ec2,*/[&host2](const error_code&)->void{std::cout << "\n\n\nTHIS is Handler: "+host2+"\n\n\n";});
  //
  //client.close();
  }
  catch(bstcon::system::exception& e){
    std::cout << e.what() << std::endl;
  }
  
  return 0;
}

