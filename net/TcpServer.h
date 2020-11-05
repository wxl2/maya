//
// Created by wxl on 2020/11/4.
//

#ifndef MAYA_TCPSERVER_H
#define MAYA_TCPSERVER_H

#include "../base/nocopyable.h"
#include "../base/Types.h"
#include "TcpConnection.h"
#include "Callbacks.h"
#include "Acceptor.h"
#include <atomic>
#include <map>
#include <memory>

namespace maya{
namespace net{

    class Acceptor;
    class EventLoop;
    class EventLoopThreadPool;

    class TcpServer:nocopyable
    {
    public:
        TcpServer(EventLoop* loop,const InetAddress& listenAddr);

        void start();

        void setConnectionCallback(const ConnectionCallback& cb)
        { connectionCallback_=cb;}

        void setMessageCallback(const MessageCallback& cb)
        {messageCallback_=cb;}

        void setThreadNum(int numThreads);
    private:
        void newConnection(int sockfd,const InetAddress& peerAddr);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);

        typedef std::map<std::string,TcpConnectionPtr> ConnectionMap;

        EventLoop* loop_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;
        std::unique_ptr<EventLoopThreadPool> threadPool_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        bool started_;
        int nextConnId_;
        ConnectionMap connections_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_TCPSERVER_H
