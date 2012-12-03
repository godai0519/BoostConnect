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

    typedef boost::shared_ptr<bstcon::connection_type::connection_base> connection_ptr;
    typedef boost::shared_ptr<bstcon::response> response_type;

    typedef boost::function<void(connection_ptr,error_code)> ConnectionHandler;
    typedef boost::function<void(response_type,error_code)> ChunkHandler;
    typedef boost::function<void(response_type,error_code)> EndHandler;
    
    connection_base();
    virtual ~connection_base();

    //通信開始(オーバーライド必須)
    virtual connection_ptr connect(const std::string&, ConnectionHandler) = 0;
    virtual connection_ptr connect(const endpoint_type&, ConnectionHandler) = 0;
    
    virtual response_type send(
        boost::shared_ptr<boost::asio::streambuf>,
        EndHandler = [](response_type,error_code)->void{},
        ChunkHandler = [](response_type,error_code)->void{}
        ) = 0;

    virtual void close();
    inline const response_type& get_response() const;

protected:
    class reader : boost::noncopyable{
    private:
        typedef connection_base::ChunkHandler ChunkHandler;
        typedef boost::function<void(error_code)> EndHandler;

        boost::asio::streambuf read_buf_;
        response_type response_;

    public:
        //ctor & dtor
        reader();
        reader(response_type& response);
        virtual ~reader();

        //Util
        const response_type& get_response() const;
        const response_type& reset_response();
        bool is_chunked() const;

        //
        // 共通メンバ関数
        //
    protected:
        //レスポンスヘッダ読み込み
        const int read_header(const std::string& source);

        //チャンクを参照な引数から返し，チャンク文字列の長さを返す
        const int chunk_parser(const std::string& source,std::size_t& chunk);

        //
        // 同期
        //
    public:
        template<class Socket>
        void read_starter(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler);

    protected:
        //chunkを持っている同期通信
        //チャンクサイズの表示行を読み出す
        template<class Socket>
        void read_chunk_size(Socket& socket,EndHandler handler,ChunkHandler chunk_handler);
    
        //chunk指定に基づいて処理
        template<class Socket>
        void read_chunk_body(Socket& socket,const std::size_t chunk,EndHandler handler,ChunkHandler chunk_handler);

        //
        // 非同期
        //
    public:
        //非同期読み出し開始
        template<class Socket>
        void async_read_starter(Socket& socket, EndHandler end_handler, ChunkHandler chunk_handler);

    protected:
        //ヘッダー処理，処理を各系統へ渡す．
        template<class Socket>
        void async_read_header(Socket& socket,const error_code& ec,const std::size_t,EndHandler handler,ChunkHandler chunk_handler);
    
        //ここに来る前に最後まで読み切ってるはず
        template<class Socket>
        void async_read_all(Socket& socket,const error_code& ec,const std::size_t size,EndHandler handler);
        
        //チャンク行を読み込み終えてるはずなので，チャンク量を読み出し．
        //(なんか魔導書と似ちゃってるような)
        template<class Socket>
        void async_read_chunk_size(Socket& socket,const error_code& ec,const std::size_t,EndHandler handler,ChunkHandler chunk_handler);

        //チャンクの表示量を読みだし終えてるはずだけど．
        template<class Socket>
        void async_read_chunk_body(Socket& socket,const std::size_t chunk,const error_code& ec,const std::size_t,EndHandler handler,ChunkHandler chunk_handler);

        //最後，ここまで．
        template<class Socket>
        void async_read_end(Socket& socket,const error_code &ec,const std::size_t,EndHandler handler);
    };

    boost::shared_ptr<application_layer::socket_base> socket_;
    std::unique_ptr<connection_base::reader> reader_;
};

template <class Devide>
class connection_common : public connection_base, public boost::enable_shared_from_this<Devide>{};

} // namespace connection_type
} // namespace bstcon

#ifdef BOOSTCONNECT_LIB_BUILD
#include "../../../src/connection_type/connection_base.cpp"
#endif

#endif
