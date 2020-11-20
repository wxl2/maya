//
// Created by wxl on 2020/11/4.
//

#ifndef MAYA_TCPCONNECTION_H
#define MAYA_TCPCONNECTION_H

#include "base/nocopyable.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "Callbacks.h"

#include <memory>
#include <functional>
struct tcp_info;
namespace maya{
namespace net{

    class TcpConnection;
    class Channel;
    class EventLoop;
    class Socket;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    class TcpConnection:nocopyable,
                        public std::enable_shared_from_this<TcpConnection>
    {
    public:
        TcpConnection(EventLoop* loop,const string& name,int sockfd,
                      const InetAddress localAddr,
                      const InetAddress perrAddr);
        ~TcpConnection();

        EventLoop* getLoop()const{return loop_;}
        const string& name() const{return name_;}
        const InetAddress& localAddress() const{return localAddr_;}
        const InetAddress& peerAddress() const{return peerAddr_;}
        bool connected() const{return state_==kConnected;}
        bool disconnected() const{return state_==kDisconnected;}

        bool getTcpInfo(struct tcp_info*) const;
        string getTcpInfoString()const;

        void send(const void* message,size_t len);
        void send(const std::string& message);
        void send(Buffer* message);
        void shutdown();
        //强制关闭
        void forceClose();
        void setTcpNoDelay(bool on);

        void startRead();
        void stopRead();
        bool isReading() const{return reading_;}

        void setConnectionCallback(const ConnectionCallback& cb)
        {connectionCallback_=cb;}
        void setMessageCallback(const MessageCallback& cb)
        {messageCallback_=cb;}
        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {writeCompleteCallback_=cb;}
        void setHighWaterMarkCallback(const HighWaterMarkCallback& cb)
        {highWaterMarkCallback_=cb;}

         Buffer* inputBuffer()
         {
             return &inputBuffer_;
         }

         Buffer* outputBuffer()
         {
             return &outputBuffer_;
         }


        void setCloseCallback(const CloseCallback& cb)
        {closeCallback_=cb;}

        // called when TcpServer accepts a new connection 连接到来
        void connectEstablished();   // should be called only once
        // called when TcpServer has removed me from its map 连接断开
        void connectDestroyed();  // should be called only once

    private:
        ///正在连接,已经连接,已经关闭连接,正在关闭连接
        enum StateE{kConnecting,kConnected,kDisconnected,kDisconnecting};
        void setState(StateE e){state_=e;}
        void handleRead(Timestamp receiveTime);
        ///内核发送缓冲区有空间了，回调该函数
        void handleWrite();
        void handleClose();
        void handleError();
        void sendInLoop(const std::string& message);
        void sendInLoop(const void* message,size_t len);
        void shutdownInLoop();

        void forceCloseInLoop();
        const char* stateToString() const;
        void startReadInLoop();
        void stopReadInLoop();

       EventLoop* loop_;
       std::string name_;
       StateE state_;
       bool reading_;
       std::unique_ptr<Socket> socket_;
       std::unique_ptr<Channel> channel_;
       InetAddress localAddr_;
       InetAddress peerAddr_;

       //FIXME:采用level tigger 即条件触发

       //当有连接到来或者断开时回调,传入的函数应该处理连接到来的链接和连接断开两种情况
       ConnectionCallback connectionCallback_;
       MessageCallback messageCallback_;
       //低水位回调,即数据都写入内核缓冲区的回调
       WriteCompleteCallback writeCompleteCallback_;
       //高水位回调,如果输出缓冲区长度打与用户所指定的大小即highWaterMark_中的值,便回调highWaterMarkCallback_
       HighWaterMarkCallback highWaterMarkCallback_;
       CloseCallback closeCallback_;
       size_t highWaterMark_;
       Buffer inputBuffer_;
       Buffer outputBuffer_;
    };
}//namespace net
}//namespace maya



#endif //MAYA_TCPCONNECTION_H
