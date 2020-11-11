//
// Created by wxl on 2020/11/5.
//

#include "TcpClient.h"
#include "base/Logging.h"
#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <stdio.h>

using namespace maya;
using namespace maya::net;

namespace maya{
namespace net{
namespace detail{
    void removeConnection(EventLoop* loop,const TcpConnectionPtr& conn)
    {
        loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
    }

    void removeConnector(const ConnectorPtr& connector)
    {

    }
}//namespace detail
}//namespace net
}//namespace maya

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr, const string &nameArg)
:loop_(CHECK_NOTNULL(loop)),
connector_(new Connector(loop,serverAddr)),
name_(nameArg),
connectionCallback_(defaultConnectionCallback),
messageCallback_(defaultMessageCallback),
retry_(false),
connect_(true),
nextConnId_(1)
{
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this,_1));
    LOG_INFO<<"TcpClient::TcpClient["<<name_<<"] connector "<<get_pointer(connector_);
}

TcpClient::~TcpClient()
{
    LOG_INFO << "TcpClient::~TcpClient[" << name_
             << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    bool unique= false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        unique=connector_.unique();
        conn=connection_;
    }
    if(conn)
    {
        if(loop_!=conn->getLoop())
            return;

        CloseCallback cb=std::bind(&detail::removeConnection,loop_, _1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback,conn,cb));
        if(unique)
        {
            conn->forceClose();
        }
    }
    else
    {
        connector_->stop();
        loop_->runAfter(1,std::bind(&detail::removeConnector,connector_));
    }
}

void TcpClient::connect()
{
    LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
             << connector_->serverAddress().toIpPort();
    connect_= true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_= false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_= false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf,sizeof(buf),":%s#%d",peerAddr.toIpPort().c_str(),nextConnId_);
    ++nextConnId_;
    string connName=name_+buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_,connName,sockfd,localAddr,peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this,_1));
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_=conn;
    }

    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        std::unique_lock<std::mutex> lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_)
    {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->serverAddress().toIpPort();
        connector_->restart();
    }
}

