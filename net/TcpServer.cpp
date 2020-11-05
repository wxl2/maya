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

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf,sizeof(buf),"#%d",nextConnId_);
    ++nextConnId_;
    std::string connName=name_+buf;

    LOG_INFO<<"TcpServer::newConnection["<<name_<<"] - new connection ["
    <<connName<<"] from"<<peerAddr.toIpPort();

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    EventLoop* ioLoop=threadPool_->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
    connections_[connName]=conn;
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    LOG_INFO<<"TcpServer::removeConnection ["<<name_<<
    "] - connection"/*<<conn->name()FIXME*/;
    //size_t n = connections_.erase(conn->name());
   // assert(n==1);
    //(void )n;
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    LOG_INFO<<"TcpServer::removeConnectionInLoop ["<<name_<<"] - connection "/*<<conn->name()*/;
//    ssize_t n = connections_.erase(conn->name);
    // assert(n==1);
    //(void )n;
//    EventLoop* ioLoop=conn.getLoop();
//    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
}
