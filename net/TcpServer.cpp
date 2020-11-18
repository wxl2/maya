//
// Created by wxl on 2020/11/4.
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"

using namespace maya;
using namespace maya::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg, Option option)
:loop_(CHECK_NOTNULL(loop)),
ipPort_(listenAddr.toIpPort()),
name_(nameArg),
acceptor_(new Acceptor(loop,listenAddr,option==kReusePort)),
threadPool_(new EventLoopThreadPool(loop,nameArg)),
connectionCallback_(defaultConnectionCallback),
messageCallback_(defaultMessageCallback),
started_(0),
nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,_1,_2));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE<<"TcpServer::~TcpServer ["<<name_<<"] destructing";
    for(auto& it:connections_)
    {
        TcpConnectionPtr conn(it.second);
        it.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0<=numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(started_==0)
    {
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listenning());
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    loop_->assertInLoopThread();
    char buf[64];
    snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName=name_+buf;

    LOG_INFO<<"TcpServer::newConnection["<<name_<<"] - new connection ["
    <<connName<<"] from"<<peerAddr.toIpPort();

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    EventLoop* ioLoop=threadPool_->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
    connections_[connName]=conn;
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,_1));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
//    loop_->assertInLoopThread();
//    LOG_INFO<<"TcpServer::removeConnection ["<<name_<<
//    "] - connection"/*<<conn->name()FIXME*/;
    //size_t n = connections_.erase(conn->name());
   // assert(n==1);
    //(void )n;
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    LOG_INFO<<"TcpServer::removeConnectionInLoop ["<<name_<<"] - connection "<<conn->name();
    ssize_t n = connections_.erase(conn->name());
    assert(n==1);
    (void )n;
    EventLoop* ioLoop=conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
}
