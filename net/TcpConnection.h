//
// Created by wxl on 2020/11/4.
//

#ifndef MAYA_TCPCONNECTION_H
#define MAYA_TCPCONNECTION_H

#include "../base/nocopyable.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "Callbacks.h"

#include <memory>
#include <functional>

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
        void setCloseCallback(const CloseCallback& cb)
        {closeCallback_=cb;}

        void send(const void* message,size_t len);
        void send(const std::string& message);
        void shutdown();
        void setTcpNoDelay(bool on);

        void connectEstablished();
        void connectDestroyed();

    private:
        enum StateE{kConnecting,kConnected,kDisconnected,kDisconnecting};
        void setState(StateE e){state_=e;}
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();
        void sendInLoop(const std::string& message);
        void shutdownInLoop();

       EventLoop* loop_;
       std::string name_;
       StateE state_;
       std::unique_ptr<Socket> socket_;
       std::unique_ptr<Channel> channel_;
       InetAddress localAddr_;
       InetAddress peerAddr_;
       ConnectionCallback connectionCallback_;
       MessageCallback messageCallback_;
       WriteCompleteCallback writeCompleteCallback_;
       HighWaterMarkCallback highWaterMarkCallback_;
       CloseCallback closeCallback_;
       Buffer inputBuffer_;
       Buffer outputBuffer_;
    };
}//namespace net
}//namespace maya



#endif //MAYA_TCPCONNECTION_H
