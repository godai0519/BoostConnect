#ifndef BOOSTCONNECT_LAYER_BASE
#define BOOSTCONNECT_LAYER_BASE

#include <memory>
#include <boost/smart_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>

#ifndef NO_SSL
#include <boost/asio/ssl.hpp>
#pragma comment(lib, "libeay32MDd.lib")
#pragma comment(lib, "ssleay32MDd.lib")
#endif

class layer_base : boost::noncopyable{ //最基底クラス？
public:
	layer_base(){} //一応ほかを追加して消す
	virtual ~layer_base(){}
	
	typedef boost::asio::io_service io_service;
	typedef io_service::work work;
	typedef boost::asio::ip::tcp::socket tcp_socket;
#ifndef NO_SSL
	typedef boost::asio::ssl::stream<tcp_socket> ssl_socket;
	typedef boost::asio::ssl::context context;
#endif
	typedef boost::system::error_code error_code;
	typedef boost::asio::ip::tcp::endpoint endpoint;

	typedef boost::function<void (error_code)> ConnectHandler;
	//Handlerは追加していく

	virtual error_code& async_connect() = 0;
	virtual void async_write() = 0;
	virtual void async_write_some() = 0;
	virtual void async_read() = 0;
	virtual void async_read_some() = 0;

	virtual error_code& sync_connect(error_code&,endpoint&) = 0;
	virtual error_code& sync_write(boost::asio::streambuf&,error_code&) = 0;
	virtual error_code& sync_read(boost::asio::streambuf&,error_code&) = 0;

	//うまく実装できないんです．
	//template<class Delim>
	//virtual error_code& sync_read_some(boost::asio::streambuf&,const Delim&,error_code&) = 0;

	virtual error_code& close(error_code& ec) = 0;

//protected:
//	boost::scoped_ptr<work> worker;
//	void worker_set(io_service& io_service){worker.reset(new work(io_service));}
//	void worker_end(){worker.reset();};
};

#endif
