BoostConnect
============
[twit-library](https://github.com/godai0519/twit-library)とかいう駄目駄目のOAuthライブラリの通信部分を分割し，リポジトリとして公開してみたものです．  
それ以上でもそれ以下でもありません．

通信部分の安定化と汎用化を求めて設計されたBoostConnectは，  
任意のポート・アドレスに接続する事ができるクライアントと，  
任意のHTTPサーバを立てることが出来るサーバーを提供しています．  
(本当はもっと機能を増やしたかったりしますが)

必要なもの
----------
+   [Boost C++ Library](http://www.boost.org/):
      大部分はこのライブラリに依存しています．ビルドしてパスを通して置いてください．
      
+   [OpenSSL](http://www.openssl.org/)
      SSL/TLS拡張を行う場合のみ，パスを通しておいてください．  
      `#define USE_SSL_BOOSTCONNECT`によってSSLが有効になります．

SSL通信をしない場合はOpenSSLは必要ありません！

使い方
-------
使い方は[sample.cpp](https://github.com/godai0519/BoostConnect/blob/master/sample/sample.cpp)を見ると解りやすいかも知れません．

簡単に説明すると，
+   `bstcon::client`がクライアント
+   `bstcon::server`がサーバー
用のクラスです．

どちらもコンストラクタで`boost::asio::io_service`を渡します．  
この時，第二引数に`boost::asio::ssl::context`を渡すと暗黙的にSSL通信を行うことになります．

サンプルの詳細な説明は以下に提示しますから，見るとわかるかも知れません．

クライアント
-----------
クライアントとしての使い方は以下のとおりです．  
それぞれのサンプルのコメントを全て外すと，SSL通信が確立します．  
SSL通信を行う場合は全ての#includeの前に，`#define USE_SSL_BOOSTCONNECT`を宣言してください．

+   共通部品

        boost::asio::io_service io_service;
        // boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);
        boost::shared_ptr<boost::asio::streambuf> request_buf(new boost::asio::streambuf());
        {
         std::ostream os(request_buf.get());
          os << "GET / HTTP/1.1\r\n";
          os << "Host: "+host+"\r\n";
          os << "Connection: close\r\n";
          os << "\r\n";
        }

+   同期通信

        bstcon::client c(
          io_service,
          // ctx,
          bstcon::connection_type::sync
          );
        
        const boost::shared_ptr<bstcon::response> response = 
          client(
            "google.co.jp",
            request_buf,
            [](const boost::shared_ptr<bstcon::response>,const error_code&)->void{std::cout << "Connection End" << std::endl;} /*同期通信ならこのハンドラーを渡さなくても勝手に何とかしてくれます*/
          );
    
+   非同期通信

        bstcon::client c(
          io_service,
          // ctx,
          bstcon::connection_type::async
          );
        
         
        client(
          "google.co.jp",
          request_buf,
          [](const boost::shared_ptr<bstcon::response> response,const error_code&)->void{std::cout << "Connection End" << std::endl;}
        );
        
        io_service.run();
    
ん？違い？  
多分クライアントの第二引数(SSLなら第三引数)と，非同期開始する`io_service.run();`だけだと思うよ．  
なるべくライブラリのユーザが使いやすいように，ポインタとかで同じように扱えるようになってる．

サーバー
---------
サーバーとしての使用は今のところHTTP通信としてのみです．  
ただHTTP通信のボディにデータを載せてやりとりするチャット(あるのかな？)とかには使用できるかも知れません．

    boost::asio::io_service io_service;
    // boost::asio::ssl::context ctx(io_service,boost::asio::ssl::context_base::sslv3_client);
    
    // ctxの設定(ctx Setting)
    
    bstcon::server service(io_service,ctx,5600);
    service.start(
      [](const request_type& req,response_type& res)
      {
        res.status_code = 200;
        res.http_version = "1.1";
        res.status_message = "OK";
        res.body = "<html><body>Test Response</body></html>";
        res.header["Content-Length"] = (boost::format("%d") % res.body.size()).str();
      }
    );

上のコードでは常に`Test Response`と表示するレスポンスを返します．  
どんなパスにリクエストが来ても同じ物を返すだけの例．  
`req.uri`のパスに応じたローカルファイルの内容を`res.body`に流し込めば簡単なHTTPサーバーとしても動きます．

"namespace bstcon"について
--------------------------
由来は BoostConnect -> BstConnect -> BstCon -> bstcon です．  
namespaceなどは，なんとなく即席で決めたので，何か既存のライブラリとの衝突などがあれば，リクエストをください．多分対処します．  
