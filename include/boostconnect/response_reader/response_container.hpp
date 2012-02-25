//
// response_container.hpp
// ~~~~~~~~~~
//
// レスポンスをまとめておいておく
//

#ifndef TWIT_LIB_PROTOCOL_RESREADER_RESPONSE_CONTAINER
#define TWIT_LIB_PROTOCOL_RESREADER_RESPONSE_CONTAINER

#include <map>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lexical_cast.hpp>

namespace oauth{
namespace protocol{
namespace response_reader{

class response_container{
public:
	typedef std::map<std::string,std::string> header_type;
	typedef std::string body_type;
	
	typedef boost::system::error_code error_code;
	typedef boost::function<void (const error_code&)> ReadHandler;

	response_container(){}
	virtual ~response_container(){}

	bool is_chunked() const {return (header_.find("Transfer-Encoding")==header_.end()) ? false : (header_.at("Transfer-Encoding")=="chunked");}
	
	//
	// 共通
	//
public:
	int status_code_;
	std::string http_version_;
	std::string status_message_;
	header_type header_;
	body_type body_;

protected:
	//少なくとも\r\n\r\nまで収納されていると仮定
	template<class T>
	const int read_header(const T& source)
	{		
		namespace qi = boost::spirit::qi;

		const auto header_rule = 
			"HTTP/" >> +(qi::char_ - " ") >> " " >> qi::int_ >> " " >> +(qi::char_ - "\r\n") >> ("\r\n") //一行目
			>> *(+(qi::char_ - ": ") >> ": " >> +(qi::char_ - "\r\n") >> "\r\n") //二行目をmapに
			>> ("\r\n"); //"\r\n\r\n"まで

		std::string::const_iterator it = source.cbegin();
		qi::parse(it,source.cend(),header_rule,http_version_,status_code_,status_message_,header_);
		
		return it - source.cbegin();
	}

	//
	// 同期
	//
public:
	template<class Socket>
	void read_starter(Socket& socket,/*error_code& ec,*/ReadHandler handler)
	{
		//ヘッダーのみ
		boost::asio::read_until(socket,read_buf,"\r\n\r\n"/*,ec*/);
		read_buf.consume(
			read_header((std::string)boost::asio::buffer_cast<const char*>(read_buf.data()))
		);
		
		if(is_chunked())
		{
			//syncなchunked
			//処理をチャンク用の関数に投げるから適当
			read_chunk_size(socket,handler);
			return;
		}
		
		//読み込みを試行
		//読み込めたらそれをbodyに追加して，エラーコードがどうであれhandlerを呼び出す
		//エラーコードがeof以外だったらthrowする
		error_code ec;
		if(header_.find("Content-Length")==header_.end())
		{
			while(boost::asio::read(socket,read_buf,/*boost::asio::transfer_at_least(1)*/boost::asio::transfer_all(),ec));
			//std::cout << ec.message();
			body_ +=  boost::asio::buffer_cast<const char*>(read_buf.data());
			read_buf.consume(read_buf.size());
		}
		else
		{
			//ここにきたなら"Content-Length"がありますよね
			const size_t content_length = boost::lexical_cast<size_t>(header_.at("Content-Length"));
			boost::asio::read(socket,read_buf,
				boost::asio::transfer_at_least(content_length-boost::asio::buffer_size(read_buf.data())),
				ec
				);
			body_ += ((std::string)boost::asio::buffer_cast<const char*>(read_buf.data())).substr(0,content_length);
			read_buf.consume(content_length);
		}

		handler(ec);
		if(ec && ec!=boost::asio::error::eof && ec.value()!=0x140000DB)
		{
			boost::asio::detail::throw_error(ec,"read_starter");
		}
		return;


		//とりあえず全部読み込み		
		//while(boost::asio::read(socket,read_buf,boost::asio::transfer_at_least(1),ec));
		//if(ec && ec != boost::asio::error::eof) return ec;

		//全部std::stringへ
		//std::string response = boost::asio::buffer_cast<const char*>(read_buf.data());
		//read_buf.consume(read_buf.size());

		//さてヘッダーを読み込みつつ不要を消す
		//response.erase(0,read_header(response));

		//で，ボディっと
		//body_.append(response);
		
		//error_code ec;
		//handler(ec);
		//return/* ec*/;
	}

protected:	
	template<class Socket>
	void read_chunk_size(Socket& socket,ReadHandler handler)
	{
		error_code ec;
		boost::asio::read_until(socket,read_buf,"\r\n",ec);
		if(ec && ec!=boost::asio::error::eof)
		{
			handler(ec);
			boost::asio::detail::throw_error(ec,"sync_chunk_read");
			//例外！
		}

		std::size_t chunk;
		read_buf.consume(chunk_parser((std::string)boost::asio::buffer_cast<const char*>(read_buf.data()),chunk));
		//chunk量+"\r\n"まで，read_bufを消し去った
		
		//chunkが0 => bodyの終了
		if(chunk == 0)
		{
			handler(ec);
			return;
		}

		//そのチャンク表示でbodyのreadを試みる．
		read_chunk_body(socket,chunk,handler);
		return;
	}
	
	template<class Socket>
	void read_chunk_body(Socket& socket,const std::size_t chunk,ReadHandler handler)
	{
		error_code ec;
		boost::asio::read(socket,read_buf,
			boost::asio::transfer_at_least(chunk+2-boost::asio::buffer_size(read_buf.data())),
			ec
			);
		
		//読み込んだところに chunk量+"\r\n" がない場合はエラとして排除
		if(boost::asio::buffer_size(read_buf.data()) < chunk + 2)
		{
			handler(ec);
			boost::asio::detail::throw_error(ec,"sync_chunk_less");
			//例外！
		}

		body_.append(boost::asio::buffer_cast<const char*>(read_buf.data()),chunk/*+2*/);
		read_buf.consume(chunk+2); //流す

		read_chunk_size(socket,handler);
		return;
	}
/*	template<class Socket>
	void read_end(Socket& socket,error_code &ec,ReadHandler handler)
	{
		//boost::asio::read(socket,read_buf,boost::asio::transfer_at_least(1)/*,ec*//*);
		//if(ec == boost::asio::error::eof || read_buf.size() == 0)
		//{
			handler(ec);
			return/* ec*//*; //空なら終わりだ．
		//}
		//return/* ec*//*; //ここはエラー
	}*/


	//
	// 非同期
	//
public:
	template<class Socket>
	void async_read_starter(Socket& socket,ReadHandler handler)
	{
		//ただわかりやすくしただけ．渡し逃げ．
		boost::asio::async_read_until(socket,
			read_buf,
			"\r\n\r\n",
			boost::bind(&response_container::async_read_header<Socket>,this,
				boost::ref(socket),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				handler));

		return;
	}

protected:
	template<class Socket>
	void async_read_header(Socket& socket,const error_code& ec,const std::size_t,ReadHandler handler)
	{
		//レスポンスが帰ってこない？
		if(read_buf.size()==0)
		{
			handler(ec);
			boost::asio::detail::throw_error(ec,"async_not_response");
			//例外！
		}

		read_buf.consume(
			read_header((std::string)boost::asio::buffer_cast<const char*>(read_buf.data()))
		);

		if(is_chunked())
		{
			//chunkedなasync通信
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n",
				boost::bind(&response_container::async_read_chunk_size<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}
		else if(header_.find("Content-Length")==header_.end())
		{
			//Content-Lengthが見つからないasync通信
			//終了条件は暗示的にtransfer_all()
			boost::asio::async_read(socket,
				read_buf,
				boost::bind(&response_container::async_read_all<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}
		else
		{
			//Content-LengthをもとにReadを行う，async通信
			//ここにきたなら"Content-Length"がありますよね
			boost::asio::async_read(socket,
				read_buf,
				boost::asio::transfer_at_least(boost::lexical_cast<size_t>(header_.at("Content-Length"))-boost::asio::buffer_size(read_buf.data())),
				boost::bind(&response_container::async_read_all<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}
				
		return;
	}
	
	//読めるだけ読み込む
	template<class Socket>
	void async_read_all(Socket& socket,const error_code& ec,const std::size_t size,ReadHandler handler)
	{
		if(read_buf.size()==0) 
		{
			handler(ec);
			boost::asio::detail::throw_error(ec,"async_not_response");
		}
		else
		{
			body_.append(boost::asio::buffer_cast<const char*>(read_buf.data()));
			read_buf.consume(read_buf.size());

			handler(ec);

			/*
			//とりあえず最後まで読み込むのです
			boost::asio::async_read(socket,
				read_buf,
				boost::bind(&response_container::async_read_all<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
			*/
		}
				
		return;
	}
		
	/*template<class Socket>
	void async_read_body(Socket& socket,const error_code& ec,const std::size_t,ReadHandler handler)
	{
		if(read_buf.size()==0) return; //空？
		else
		{
			body_.append(boost::asio::buffer_cast<const char*>(read_buf.data()));
			read_buf.consume(read_buf.size());
		}

		async_read_starter(socket);
		
		return;
	}
	*/
	//なんか魔導書と似ちゃってるような
	template<class Socket>
	void async_read_chunk_size(Socket& socket,const error_code& ec,const std::size_t,ReadHandler handler)
	{
		if(read_buf.size()==0)
		{
			//ヘッダー読み込んだから半端は有るはず
			//ということは，ここに来るとマズい
			handler(ec);
			boost::asio::detail::throw_error(ec,"async_read_chunk");
		}
		else if(read_buf.size()<=2)
		{
			//chunk量+"\r\n"ないからもう一回Readしてみようか
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n",
				boost::bind(&response_container::async_read_chunk_size<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}
		else
		{
			//chunk入ってるので，chunkを読み込もう
			//chunk量+"\r\n"まで，read_bufも消し去る
			std::size_t chunk;
			read_buf.consume(chunk_parser((std::string)boost::asio::buffer_cast<const char*>(read_buf.data()),chunk));

			//chunk量読み出し
			boost::asio::async_read(socket,
				read_buf,
				boost::asio::transfer_at_least(chunk+2-boost::asio::buffer_size(read_buf.data())),
				boost::bind(&response_container::async_read_chunk_body<Socket>,this,
					boost::ref(socket),
					chunk,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}

		return;
	}
	template<class Socket>
	void async_read_chunk_body(Socket& socket,const std::size_t chunk,const error_code& ec,const std::size_t,ReadHandler handler)
	{
		if(read_buf.size()==0)
		{
			//ないんだけど？
			handler(ec);
			boost::asio::detail::throw_error(ec,"async_read_body");
		}
		else if(chunk == 0) //終わったけど
		{
			boost::asio::async_read(socket,
				read_buf,
				boost::bind(&response_container::async_read_end<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}
		else
		{
			//さて本体
			if(boost::asio::buffer_size(read_buf.data()) < chunk + 2) return;

			//後ろに追加
			body_.append(boost::asio::buffer_cast<const char*>(read_buf.data()),chunk);
			read_buf.consume(chunk+2); //流す

			//chunk取得にもどるよ
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n",
				boost::bind(&response_container::async_read_chunk_size<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					handler));
		}
	}
	template<class Socket>
	void async_read_end(Socket& socket,const error_code &ec,const std::size_t,ReadHandler handler)
	{
		if(read_buf.size() != 0)
		{
			//終わってない…だと？
			handler(ec);
			boost::asio::detail::throw_error(ec,"async_not_end");
		}

		handler(ec);
		return; //空なら終わりだ
		/*
		//ヘッダーから呼び出し始める
		boost::asio::async_read_until(socket,
			read_buf,
			"\r\n\r\n",
			boost::bind(&response_container::async_read_header<Socket>,this,
				boost::ref(socket),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				handler));*/
	}

private:
	//チャンクを参照な引数から返し，チャンク文字列の長さを返す
	template<class T>
	const int chunk_parser(const T& source,std::size_t& chunk)
	{
		namespace qi = boost::spirit::qi;
		const auto header_rule = (qi::hex[boost::phoenix::ref(chunk) = qi::_1] - "\r\n") >> *(qi::char_ - "\r\n") >> ("\r\n");

		T::const_iterator it = source.cbegin();
		qi::parse(it,source.cend(),header_rule,chunk);

		return it-source.cbegin();
	}
	boost::asio::streambuf read_buf;
};

} // namespace application_layer
} // namespace protocol
} // namespace oauth

#endif
