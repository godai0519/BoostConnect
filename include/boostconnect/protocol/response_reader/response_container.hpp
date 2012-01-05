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

	response_container(){}
	virtual ~response_container(){}

	bool is_chunked() const {return (header_.find("Transfer-Encoding")==header_.end()) ? false : (header_.at("Transfer-Encoding")=="chunked");}
	
	//
	// 共通
	//

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
	error_code& read_starter(Socket& socket,error_code& ec)
	{
		//とりあえず全部読み込み		
		while(boost::asio::read(socket,read_buf,boost::asio::transfer_at_least(1),ec));
		if(ec && ec != boost::asio::error::eof) return ec;

		//全部std::stringへ
		std::string response = boost::asio::buffer_cast<const char*>(read_buf.data());
		read_buf.consume(read_buf.size());

		//さてヘッダーを読み込みつつ不要を消す
		response.erase(0,read_header(response));

		//で，ボディっと
		body_.append(response);

		return ec;
	}
/*
protected:
	//少なくとも\r\n\r\nまで収納されていると仮定
	bool read_head(boost::asio::streambuf& buf)
	{
		//ヘッダ抜き出し
		std::string header = boost::asio::buffer_cast<const char*>(buf.data());
		header.erase(header.find("\r\n\r\n")+4);
		buf.consume(header.size()); //ヘッダ部切り捨て(レスポンスボディが短すぎると被っちゃうよ？)

		//パース
		namespace qi = boost::spirit::qi;

		const auto header_rule = 
			"HTTP/" >> +(qi::char_ - " ") >> " " >> qi::int_ >> " " >> +(qi::char_ - "\r\n") >> ("\r\n") //一行目
			>> *(+(qi::char_ - ": ") >> ": " >> +(qi::char_ - "\r\n") >> "\r\n") //二行目をmapに
			>> *("\r\n") >> *qi::eol; //改行が残ってたら全部スルー
		
		std::string::const_iterator it = header.cbegin();
		bool r = qi::parse(it,header.cend(),header_rule,http_version_,status_code_,status_message_,header_);
		if (it != header.end()) return false;
		return r;
	}

	void read_body(boost::asio::streambuf& buf)
	{
		body_ += boost::asio::buffer_cast<const char*>(buf.data());
		//bufの賞味期限切れかと・・・美味しくないよ？

		bool cutable = header_.find("Content-Length")!=header_.end();
		if(cutable) body_.erase(boost::lexical_cast<int>(header_.at("Content-Length")));

		return;
	}*/

	//
	// 非同期
	//
public:
	template<class Socket>
	void async_read_starter(Socket& socket)
	{
		//ただわかりやすくしただけ．渡し逃げ．
		boost::asio::async_read_until(socket,
			read_buf,
			"\r\n\r\n",
			boost::bind(&response_container::async_read_header<Socket>,this,
				boost::ref(socket),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

		return;
	}

protected:
	template<class Socket>
	void async_read_header(Socket& socket,const error_code& ec,const std::size_t)
	{
		if(read_buf.size()==0) return; //きっとレスポンス無いタイプ
		read_buf.consume(
			read_header((std::string)boost::asio::buffer_cast<const char*>(read_buf.data()))
		);

		if(is_chunked())
		{
			//asyncなchunked
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n",
				boost::bind(&response_container::read_chunk_size<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

			return;
		}

		//ここにきたなら"Content-Length"がありますよね
		boost::asio::async_read(socket,
			read_buf,
			boost::asio::transfer_at_least(boost::lexical_cast<size_t>(header_.at("Content-Length"))-boost::asio::buffer_size(read_buf.data())),
			boost::bind(&response_container::async_read_body<Socket>,this,
				boost::ref(socket),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
				
		return;
	}
	
	template<class Socket>
	void async_read_body(Socket& socket,const error_code& ec,const std::size_t)
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

	//なんか魔導書と似ちゃってるような
	template<class Socket>
	void read_chunk_size(Socket& socket,const error_code& ec,const std::size_t)
	{
		if(read_buf.size()==0) return; //ヘッダー読み込んだから半端は有るはず
		if(read_buf.size()<=2) //chunk量+"\r\n"ない．
		{
			//チャンク量
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n",
				boost::bind(&response_container::read_chunk_size<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else //chunkが入ってる
		{
			std::size_t chunk;
			read_buf.consume(chunk_parser((std::string)boost::asio::buffer_cast<const char*>(read_buf.data()),chunk));
			//chunk量+"\r\n"まで，read_bufを消し去った

			//chunk量読み出し
			boost::asio::async_read(socket,
				read_buf,
				boost::asio::transfer_at_least(chunk+2-boost::asio::buffer_size(read_buf.data())),
				boost::bind(&response_container::read_chunk_body<Socket>,this,
					boost::ref(socket),
					chunk,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}

		return;
	}
	template<class Socket>
	void read_chunk_body(Socket& socket,const std::size_t chunk,const error_code& ec,const std::size_t)
	{
		if(read_buf.size()==0) return; //ないんだけど？
		if(chunk == 0) //終わったけど
		{
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n\r\n",
				boost::bind(&response_container::async_read_end<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else //さて本体
		{
			if(boost::asio::buffer_size(read_buf.data()) < chunk + 2) return;

			//後ろに追加
			body_.append(boost::asio::buffer_cast<const char*>(read_buf.data()),chunk+2);
			read_buf.consume(chunk+2); //流す

			//chunk取得にもどるよ
			boost::asio::async_read_until(socket,
				read_buf,
				"\r\n\r\n",
				boost::bind(&response_container::read_chunk_size<Socket>,this,
					boost::ref(socket),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}
	template<class Socket>
	void async_read_end(Socket& socket,const error_code &ec,const std::size_t)
	{
		if(read_buf.size() == 0) return; //空なら終わりだ．

		//ヘッダーから呼び出し始める
		boost::asio::async_read_until(socket,
			read_buf,
			"\r\n\r\n",
			boost::bind(&response_container::async_read_header<Socket>,this,
				boost::ref(socket),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

private:
	//チャンクを参照な引数から返し，チャンク文字列の長さを返す
	template<class T>
	const int chunk_parser(const T& source,std::size_t& chunk)
	{
		namespace qi = boost::spirit::qi;
		const auto header_rule = (qi::hex[boost::phoenix::ref(chunk) = qi::_1] - "\r\n") >> ("\r\n");

		T::const_iterator it = source.cbegin();
		qi::parse(it,source.cend(),header_rule,chunk);

		return it-source.cbegin();
	}

	int status_code_;
	std::string http_version_;
	std::string status_message_;
	header_type header_;
	body_type body_;
	boost::asio::streambuf read_buf;
};

} // namespace application_layer
} // namespace protocol
} // namespace oauth

#endif
