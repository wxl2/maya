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
        typedef std::function<void(EventLoop*)> ThreadInitCallback;

        enum Option
        {
            kNoReusePort,
            kReusePort
        };
        TcpServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg,Option option=kNoReusePort);
        ~TcpServer();

        const string& ipPort() const{return ipPort_;}
        const string& name() const{return name_;}
        EventLoop* getLoop() const{return loop_;}

        void setThreadNum(int numThreads);
        void setThreadInitCallback(const ThreadInitCallback& cb)
        {threadInitCallback_=cb;}

        std::shared_ptr<EventLoopThreadPool> threadPool()
        {return threadPool_;}

        void start();

        void setConnectionCallback(const ConnectionCallback& cb)
        { connectionCallback_=cb;}

        void setMessageCallback(const MessageCallback& cb)
        {messageCallback_=cb;}

        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {writeCompleteCallback_=cb;}

    private:
        void newConnection(int sockfd,const InetAddress& peerAddr);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);

        typedef std::map<std::string,TcpConnectionPtr> ConnectionMap;

        EventLoop* loop_;
        const string ipPort_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;
        std::shared_ptr<EventLoopThreadPool> threadPool_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        ThreadInitCallback threadInitCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        std::atomic<int> started_;
        int nextConnId_;
        ConnectionMap connections_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_TCPSERVER_H
