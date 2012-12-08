#ifndef BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE_IPP
#define BOOSTCONNECT_CONNECTTYPE_CONNECTION_BASE_IPP

#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boostconnect/connection_type/connection_base.hpp>

namespace bstcon{
namespace connection_type{
    
//こいつらは無視の方針で(というかインターフェース)
connection_base::connection_base(){}
connection_base::~connection_base(){}

void connection_base::close()
{
    socket_->close();
}

inline const connection_base::response_type& connection_base::get_response() const 
{
    return reader_->get_response();
}

connection_base::reader::reader() : response_(new bstcon::response())
{
}
connection_base::reader::reader(response_type& response) : response_(response)
{
}
connection_base::reader::~reader()
{
}

//Util
const connection_base::response_type& connection_base::reader::get_response() const 
{
    return response_;
}
const connection_base::response_type& connection_base::reader::reset_response()
{
    response_.reset(new bstcon::response());
    return response_;
}
bool connection_base::reader::is_chunked() const
{
    return (response_->header.find("Transfer-Encoding")==response_->header.end())
        ? false
        : (response_->header.at("Transfer-Encoding")=="chunked");
}

//レスポンスヘッダ読み込み
const int connection_base::reader::read_header(const std::string& source)
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
const int connection_base::reader::chunk_parser(const std::string& source,std::size_t& chunk)
{
    namespace qi = boost::spirit::qi;
    const qi::rule<std::string::const_iterator,unsigned int()> rule = qi::hex >> qi::lit("\r\n");

    std::string::const_iterator it = source.cbegin();
    qi::parse(it,source.cend(),rule,chunk);

    return std::distance(source.cbegin(),it);
}

template<class Socket>
void connection_base::reader::read_starter(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler)
{
    //ヘッダーのみ
    boost::asio::read_until(socket,read_buf_,"\r\n\r\n");
    read_buf_.consume(
        read_header((std::string)boost::asio::buffer_cast<const char*>(read_buf_.data()))
    );
        
    error_code ec;
    if(is_chunked())
    {
        //syncなchunked
        //処理をチャンク用の関数に投げるから適当
        read_chunk_size(socket,end_handler,chunk_handler);
    }
    else if(response_->header.find("Content-Length")==response_->header.end())
    {        
        //"Content-Length"が無いから，とりあえず全部．
        while(boost::asio::read(socket,read_buf_,boost::asio::transfer_all(),ec));

        auto data = read_buf_.data();
        response_->body.append(boost::asio::buffers_begin(data),boost::asio::buffers_end(data));
        read_buf_.consume(read_buf_.size());
    }
    else
    {
        //ここにきたなら"Content-Length"がありますよね
        const size_t content_length = boost::lexical_cast<size_t>(response_->header.at("Content-Length"));
        boost::asio::read(socket,read_buf_,
            boost::asio::transfer_at_least(content_length - boost::asio::buffer_size(read_buf_.data())),
            ec
            );

        auto data = read_buf_.data();
        response_->body.append(boost::asio::buffers_begin(data),boost::asio::buffers_end(data));
        read_buf_.consume(content_length);
    }

    end_handler(ec);
    if(ec && ec!=boost::asio::error::eof && ec.value()!=0x140000DB)
    {
        boost::asio::detail::throw_error(ec,"read_starter");
    }
    return;
}

//chunkを持っている同期通信
//チャンクサイズの表示行を読み出す
template<class Socket>
void connection_base::reader::read_chunk_size(Socket& socket,EndHandler handler,ChunkHandler chunk_handler)
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
    if(chunk == 0) return;

    //そのチャンク表示でbodyのreadを試みる．
    read_chunk_body(socket,chunk,handler,chunk_handler);
    return;
}
    
//chunk指定に基づいて処理
template<class Socket>
void connection_base::reader::read_chunk_body(Socket& socket,const std::size_t chunk,EndHandler handler,ChunkHandler chunk_handler)
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

//非同期読み出し開始
template<class Socket>
void connection_base::reader::async_read_starter(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler)
{
    //ただわかりやすくしただけ．渡し逃げ．まあ，ヘッダを読み込み切ってくれれば．
    boost::asio::async_read_until(socket,
        read_buf_,
        "\r\n\r\n",
        boost::bind(&reader::async_read_header<Socket>,this,
            boost::ref(socket),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            end_handler,
            chunk_handler));

    return;
}

//ヘッダー処理，処理を各系統へ渡す．
template<class Socket>
void connection_base::reader::async_read_header(Socket& socket,const error_code& ec,const std::size_t,EndHandler handler,ChunkHandler chunk_handler)
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
        //終了条件は暗示的にtransfer_all() = 読めるだけ読み込む
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
void connection_base::reader::async_read_all(Socket& socket,const error_code& ec,const std::size_t size,EndHandler handler)
{
    if(read_buf_.size()==0) 
    {
        handler(ec);
        boost::asio::detail::throw_error(ec,"async_not_response");
    }
    else
    {
        auto data = read_buf_.data();
        response_->body.append(boost::asio::buffers_begin(data),boost::asio::buffers_end(data));
        read_buf_.consume(read_buf_.size());

        handler(ec);
    }
    //socket.close();
    return;
}
        
//チャンク行を読み込み終えてるはずなので，チャンク量を読み出し．
//(なんか魔導書と似ちゃってるような)
template<class Socket>
void connection_base::reader::async_read_chunk_size(Socket& socket,const error_code& ec,const std::size_t,EndHandler handler,ChunkHandler chunk_handler)
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
void connection_base::reader::async_read_chunk_body(Socket& socket,const std::size_t chunk,const error_code& ec,const std::size_t,EndHandler handler,ChunkHandler chunk_handler)
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
void connection_base::reader::async_read_end(Socket& socket,const error_code &ec,const std::size_t,EndHandler handler)
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

} // namespace connection_type
} // namespace bstcon

#endif
