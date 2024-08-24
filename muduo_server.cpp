/*
muduo网络库给用户提供了
TcpServer : 用于编写服务器程序
TcpClient : 用于编写客户端程序
epoll + 线程池
好处: 能够把网络I/O 的代码和业务代码区分开
*/

#include <muduo/net/TcpServer.h> 
#include <muduo/net/EventLoop.h>
#include <functional>
#include <iostream>
#include <string>
/*
基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3. 明确TcpServer 构造函数需要的参数
4.在当前服务器类的构造函数中，注册处理连接的回调函数，处理读写事件的回调函数
5.设置合适的服务端线程数量，muduo会自己划分I/O线程和worker线程
*/

class ChatServer {
public:
    ChatServer(muduo::net::EventLoop* loop , const muduo::net::InetAddress& listenAddr , 
        const std::string& nameArg ) : _server(loop , listenAddr , nameArg) , _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection , this , std::placeholders::_1 ) ); 
        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage , this , std::placeholders::_1 , 
        std::placeholders::_2 , std::placeholders::_3) ) ; 
        

        // 设置服务器端的线程数量
        _server.setThreadNum(8);  
    } 

    // 开启事件循环
    void start() {
        _server.start() ; 
    }


private:

    void onConnection(const muduo::net::TcpConnectionPtr& conn ){


        if(conn->connected() ) {
            std::cout << conn->peerAddress().toIpPort()
        << "   ->   " << conn->localAddress().toIpPort() <<  "  state:online" << std::endl ; 
    
        }   else {
            std::cout << conn->peerAddress().toIpPort()
        << "   ->   " << conn->localAddress().toIpPort() <<  "  state:offline" << std::endl ; 
            conn->shutdown() ;
            
        }
    }

    // 专门处理用户的读写事件
    void onMessage(const muduo::net::TcpConnectionPtr& conn , muduo::net::Buffer* buf , muduo::Timestamp time ) {
        std::string buffer = buf->retrieveAllAsString() ; 
        std::cout << "recv data: " << buffer << "   time:   " <<  time.toString()  << std::endl ;
        conn->send(buffer) ;  
    }   

    muduo::net::TcpServer  _server ; 
    muduo::net::EventLoop *_loop ;  // epoll
} ; 


int main(int agrc , char** argv ) {

    muduo::net::EventLoop loop ; 
    muduo::net::InetAddress addr("127.0.0.1" , 6000 ) ; 
    ChatServer server(&loop , addr , "ChatServer"); 
    
    server.start() ;  // listenfd epoll_ctl ===> epoll 



    /*
        底层调用 epoll_wait() 以阻塞的方式等待新用户连接或已连接用户的读写事件等操作
    */
    loop.loop() ; 


    return 0 ; 
}
