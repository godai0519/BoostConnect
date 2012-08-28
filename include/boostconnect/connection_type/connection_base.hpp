//
// connection_base.hpp
// ~~~~~~~~~~
//
// 同期や非同期判断のための、Boost.Asioを使用したクラス群
//

#ifndef BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE
#define BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE

#include <memory>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include "../application_layer/socket_base.hpp"
#include "../response.hpp"

namespace bstcon{
namespace connection_type{
  enum connection_type{async,sync};

class connection_base : boost::noncopyable{
public:
  typedef boost::system::error_code error_code;
  typedef boost::asio::ip::tcp::endpoint endpoint_type;
  typedef boost::function<void (const error_code&)> ReadHandler;
  typedef boost::function<void (const boost::shared_ptr<bstcon::response>,const error_code&)> EveryChunkHandler;
  typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;
  typedef boost::shared_ptr<bstcon::response> response_type;

  connection_base(){}
  virtual ~connection_base(){}

  //通信開始(オーバーライド必須)
  virtual connection_ptr operator() (const std::string&,boost::shared_ptr<boost::asio::streambuf>,ReadHandler handler = [](const error_code&)->void{},EveryChunkHandler chunk_handler = [](const error_code&)->void{}) = 0;
  virtual connection_ptr operator() (const endpoint_type&,boost::shared_ptr<boost::asio::streambuf>,ReadHandler handler = [](const error_code&)->void{},EveryChunkHandler chunk_handler = [](const error_code&)->void{}) = 0;

  inline const response_type& get_response() const { return reader_->get_response(); }

protected:
  class reader : boost::noncopyable{
  protected:
    typedef boost::shared_ptr<bstcon::response> response_type;
    typedef boost::system::error_code error_code;
    typedef boost::function<void (const error_code&)> ReadHandler;
    typedef boost::function<void (const boost::shared_ptr<bstcon::response>,const error_code&)> EveryChunkHandler;
    boost::asio::streambuf read_buf_;
    response_type response_;

  public:
    //ctor & dtor
    reader() : response_(new bstcon::response()) { }
    reader(response_type& response) : response_(response) { }
    virtual ~reader() { }

    //Util
    const response_type& get_response() const { return response_; }
    const response_type& reset_response()
    {
      response_.reset(new bstcon::response());
      return response_;
    }
    bool is_chunked() const {return (response_->header.find("Transfer-Encoding")==response_->header.end()) ? false : (response_->header.at("Transfer-Encoding")=="chunked");}

    //
    // 共通メンバ関数
    //
  protected:
    //レスポンスヘッダ読み込み
    const int read_header(const std::string& source)
    {    
      namespace qi = boost::spirit::qi;

      qi::rule<std::string::const_iterator,std::pair<std::string,std::string>> field = (+(qi::char_ - ": ") >> ": " >> +(qi::char_ - "\r\n") >> "\r\n");
      
      std::string::const_iterator it = source.cbegin();
      qi::parse(it,source.cend(),"HTTP/" >> +(qi::char_ - " ") >> " " >> qi::int_ >> " " >> +(qi::char_ - "\r\n") >> ("\r\n"),
        response_->http_version,
        response_->status_code,
        response_->status_message);

      qi::parse(it,source.cend(),*(+(qi::char_ - ": ") >> ": " >> +(qi::char_ - "\r\n") >> "\r\n"),
        response_->header);

      qi::parse(it,source.cend(),qi::lit("\r\n"));
    
      return std::distance(source.cbegin(),it);
    }
    //チャンクを参照な引数から返し，チャンク文字列の長さを返す
    template<class T>
    const int chunk_parser(const T& source,std::size_t& chunk)
    {
      namespace qi = boost::spirit::qi;
      const qi::rule<typename T::const_iterator,unsigned int()> rule = qi::hex >> qi::lit("\r\n");

    //  const auto header_rule = (qi::hex[boost::phoenix::ref(chunk) = qi::_1] - qi::lit("\r\n")) >> *(qi::char_ - qi::lit("\r\n")) >> qi::lit("\r\n");

      typename T::const_iterator it = source.cbegin();
      qi::parse(it,source.cend(),rule,chunk);

      return std::distance(source.cbegin(),it);
    }

    //
    // 同期
    //
  public:
    template<class Socket>
    void read_starter(Socket& socket,/*error_code& ec,*/ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      //ヘッダーのみ
      boost::asio::read_until(socket,read_buf_,"\r\n\r\n"/*,ec*/);
      read_buf_.consume(
        read_header((std::string)boost::asio::buffer_cast<const char*>(read_buf_.data()))
      );
    
      if(is_chunked())
      {
        //syncなchunked
        //処理をチャンク用の関数に投げるから適当
        read_chunk_size(socket,handler,chunk_handler);
        return;
      }
    
      //読み込みを試行
      //読み込めたらそれをbodyに追加して，エラーコードがどうであれhandlerを呼び出す
      //エラーコードがeof以外だったらthrowする
      error_code ec;
      if(response_->header.find("Content-Length")==response_->header.end())
      {
        while(boost::asio::read(socket,read_buf_,/*boost::asio::transfer_at_least(1)*/boost::asio::transfer_all(),ec));
        //std::cout << ec.message();
        response_->body +=  boost::asio::buffer_cast<const char*>(read_buf_.data());
        read_buf_.consume(read_buf_.size());
      }
      else
      {
        //ここにきたなら"Content-Length"がありますよね
        const size_t content_length = boost::lexical_cast<size_t>(response_->header.at("Content-Length"));
        boost::asio::read(socket,read_buf_,
          boost::asio::transfer_at_least(content_length-boost::asio::buffer_size(read_buf_.data())),
          ec
          );
        auto data = read_buf_.data();
        response_->body.append(boost::asio::buffers_begin(data),boost::asio::buffers_end(data));
        //response_->body += ((std::string)boost::asio::buffer_cast<const char*>(read_buf_.data())).substr(0,content_length);
        read_buf_.consume(content_length);
      }

      handler(ec);
      if(ec && ec!=boost::asio::error::eof && ec.value()!=0x140000DB)
      {
        boost::asio::detail::throw_error(ec,"read_starter");
      }
      return;
    }

  protected:
    //chunkを持っている同期通信
    //チャンクサイズの表示行を読み出す
    template<class Socket>
    void read_chunk_size(Socket& socket,ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      error_code ec;
      boost::asio::read_until(socket,read_buf_,"\r\n",ec);
      if(ec && ec!=boost::asio::error::eof)
      {
        handler(ec);
        boost::asio::detail::throw_error(ec,"sync_chunk_read");
        //例外！
      }

      std::size_t chunk;
      read_buf_.consume(chunk_parser((std::string)boost::asio::buffer_cast<const char*>(read_buf_.data()),chunk));
      //chunk量+"\r\n"まで，read_bufを消し去った
    
      //chunkが0 => bodyの終了
      if(chunk == 0)
      {
        handler(ec);
        return;
      }

      //そのチャンク表示でbodyのreadを試みる．
      read_chunk_body(socket,chunk,handler,chunk_handler);
      return;
    }
  
    //chunk指定に基づいて処理
    template<class Socket>
    void read_chunk_body(Socket& socket,const std::size_t chunk,ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      error_code ec;
      boost::asio::read(socket,read_buf_,
        boost::asio::transfer_at_least(chunk+2-boost::asio::buffer_size(read_buf_.data())),
        ec
        );
    
      //読み込んだところに chunk量+"\r\n" がない場合はエラとして排除
      if(boost::asio::buffer_size(read_buf_.data()) < chunk + 2)
      {
        handler(ec);
        boost::asio::detail::throw_error(ec,"sync_chunk_less");
        //例外！
      }

      response_->body.append(boost::asio::buffer_cast<const char*>(read_buf_.data()),chunk/*+2*/);
      read_buf_.consume(chunk+2); //流す

      chunk_handler(response_,ec);

      read_chunk_size(socket,handler,chunk_handler);
      return;
    }
  
    //
    // 非同期
    //
  public:
    //非同期読み出し開始
    template<class Socket>
    void async_read_starter(Socket& socket,ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      //ただわかりやすくしただけ．渡し逃げ．まあ，ヘッダを読み込み切ってくれれば．
      boost::asio::async_read_until(socket,
        read_buf_,
        "\r\n\r\n",
        boost::bind(&reader::async_read_header<Socket>,this,
          boost::ref(socket),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred,
          handler,
          chunk_handler));

      return;
    }

  protected:
    //ヘッダー処理，処理を各系統へ渡す．
    template<class Socket>
    void async_read_header(Socket& socket,const error_code& ec,const std::size_t,ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      //レスポンスが帰ってこない？
      if(read_buf_.size()==0)
      {
        handler(ec);
        boost::asio::detail::throw_error(ec,"async_not_response");
        //例外！
      }

      read_buf_.consume(
        read_header((std::string)boost::asio::buffer_cast<const char*>(read_buf_.data()))
      );

      if(is_chunked())
      {
        //chunkedなasync通信
        boost::asio::async_read_until(socket,
          read_buf_,
          "\r\n",
          boost::bind(&reader::async_read_chunk_size<Socket>,this,
            boost::ref(socket),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler,
            chunk_handler));
      }
      else if(response_->header.find("Content-Length")==response_->header.end())
      {
        //Content-Lengthが見つからないasync通信
        //終了条件は暗示的にtransfer_all()　= 読めるだけ読み込む
        boost::asio::async_read(socket,
          read_buf_,
          boost::bind(&reader::async_read_all<Socket>,this,
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
          read_buf_,
          boost::asio::transfer_at_least(boost::lexical_cast<size_t>(response_->header.at("Content-Length"))-boost::asio::buffer_size(read_buf_.data())),
          boost::bind(&reader::async_read_all<Socket>,this,
            boost::ref(socket),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler));
      }
        
      return;
    }
  
    //ここに来る前に最後まで読み切ってるはず
    template<class Socket>
    void async_read_all(Socket& socket,const error_code& ec,const std::size_t size,ReadHandler handler)
    {
      if(read_buf_.size()==0) 
      {
        handler(ec);
        boost::asio::detail::throw_error(ec,"async_not_response");
      }
      else
      {
        response_->body.append(boost::asio::buffer_cast<const char*>(read_buf_.data()));
        read_buf_.consume(read_buf_.size());

        handler(ec);
      }
        
      return;
    }
    
    //チャンク行を読み込み終えてるはずなので，チャンク量を読み出し．
    //(なんか魔導書と似ちゃってるような)
    template<class Socket>
    void async_read_chunk_size(Socket& socket,const error_code& ec,const std::size_t,ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      if(read_buf_.size()==0)
      {
        //ヘッダー読み込んだから半端は有るはず
        //ということは，ここに来るとマズい
        handler(ec);
        boost::asio::detail::throw_error(ec,"async_read_chunk");
      }
      else if(read_buf_.size()<=2)
      {
        //chunk量+"\r\n"ないからもう一回Readしてみようか
        boost::asio::async_read_until(socket,
          read_buf_,
          "\r\n",
          boost::bind(&reader::async_read_chunk_size<Socket>,this,
            boost::ref(socket),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler,
            chunk_handler));
      }
      else
      {
        //chunk入ってるので，chunkを読み込もう
        //chunk量+"\r\n"まで，read_bufも消し去る
        std::size_t chunk;
        read_buf_.consume(chunk_parser((std::string)boost::asio::buffer_cast<const char*>(read_buf_.data()),chunk));

        if(chunk == 0) //終わったけど
        {
          boost::asio::async_read(socket,
            read_buf_,
            boost::bind(&reader::async_read_end<Socket>,this,
              boost::ref(socket),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred,
              handler));
        }

        //chunk量読み出し
        boost::asio::async_read(socket,
          read_buf_,
          boost::asio::transfer_at_least(chunk+2-boost::asio::buffer_size(read_buf_.data())),
          boost::bind(&reader::async_read_chunk_body<Socket>,this,
            boost::ref(socket),
            chunk,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler,
            chunk_handler));
      }

      return;
    }

    //チャンクの表示量を読みだし終えてるはずだけど．
    template<class Socket>
    void async_read_chunk_body(Socket& socket,const std::size_t chunk,const error_code& ec,const std::size_t,ReadHandler handler,EveryChunkHandler chunk_handler)
    {
      if(read_buf_.size()==0)
      {
        //ないんだけど？
        handler(ec);
        boost::asio::detail::throw_error(ec,"async_read_body");
        //例外！
      }
      else
      {
        //さて本体
        if(boost::asio::buffer_size(read_buf_.data()) < chunk + 2) return;

        //後ろに追加
        response_->body.append(boost::asio::buffer_cast<const char*>(read_buf_.data()),chunk);
        read_buf_.consume(chunk+2); //流す

        chunk_handler(response_,ec);
        
        //chunk取得にもどるよ
        boost::asio::async_read_until(socket,
          read_buf_,
          "\r\n",
          boost::bind(&reader::async_read_chunk_size<Socket>,this,
            boost::ref(socket),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            handler,
            chunk_handler));
      }
    }

    //最後，ここまで．
    template<class Socket>
    void async_read_end(Socket& socket,const error_code &ec,const std::size_t,ReadHandler handler)
    {
      if(read_buf_.size() != 0)
      {
        //終わってない…だと？
        handler(ec);
        boost::asio::detail::throw_error(ec,"async_not_end");
      }

      handler(ec);
      return; //空なら終わりだ
    }
  };
  boost::shared_ptr<application_layer::socket_base> socket_;
  std::unique_ptr<connection_base::reader> reader_;
};

template <class Devide>
class connection_common : public connection_base, public boost::enable_shared_from_this<Devide>{};

} // namespace connection_type
} // namespace bstcon

#endif
