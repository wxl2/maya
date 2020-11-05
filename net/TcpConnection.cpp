//
// Created by wxl on 2020/11/4.
//

#include "TcpConnection.h"
#include "EventLoop.h"
#include "../base/Logging.h"
#include "SocketsOps.h"
#include "Socket.h"
#include "Channel.h"
#include <unistd.h>


using namespace maya;
using namespace maya::net;

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno=0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&saveErrno);
    if(n>0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n==0)
    {
        handleClose();
    }
    else
    {
        errno=saveErrno;
        LOG_SYSERR<<"TcpConnection::handleRead";
        handleError();
    }
}

TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd, const InetAddress localAddr,
                             const InetAddress perrAddr)
//:socket_(new Socket(sockfd)),
{

}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_TRACE<<"TcpConnection::handleClose state"<<state_;
    assert(state_==kConnected);
//FIXME    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR<<"TcpCOnnection::handleError ["<<name_<<"] - SO_ERROR = "<<err<<" "<<strerror_tl(err);
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_==kConnected);
    setState(kDisconnected);
    //FIXME channel_->disableAll();
    connectionCallback_(shared_from_this());
    loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::shutdown()
{
    if(state_==kConnected)
    {
        setState(kDisconnected);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::send(const string &message)
{
    if(state_==kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this,message));
        }
    }
}

void TcpConnection::send(const void *message, size_t len)
{
    send(std::string(static_cast<const char*>(message),len));
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {
        ssize_t n =::write(channel_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
        if(n>0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes()==0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
                if(state_==kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
            else
            {
                LOG_TRACE<<"I am going to write more data";
            }
        }
        else
        {
            LOG_SYSERR<<"TcpConnection::handleWrite";
        }
    }
    else
    {
        LOG_TRACE<<"Connection is down, no more writting";
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::sendInLoop(const string &message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote=0;
    if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0)
    {
        nwrote=::write(channel_->fd(),message.data(),message.size());
        if(nwrote>=0)
        {
            if(implicit_cast<size_t>(nwrote)<message.size())
            {
                LOG_TRACE<<"I am going to write more data";
            }
            else if(writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else
        {
            nwrote=0;
            if(errno!=EWOULDBLOCK)
            {
                LOG_SYSERR<<"TcpConnection::sendInLoop";
            }
        }
    }

    assert(nwrote>0);
    if(implicit_cast<size_t>(nwrote)<message.size())
    {
        outputBuffer_.append(message.data(),message.size()-nwrote);
        if(channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

