//
// Created by wxl on 2020/11/4.
//

#include "TcpConnection.h"
#include "EventLoop.h"
#include "../base/Logging.h"
#include "SocketsOps.h"
#include "Socket.h"
#include "Channel.h"
#include <errno.h>
#include <unistd.h>


using namespace maya;
using namespace maya::net;

void maya::net::defaultConnectionCallback(const TcpConnectionPtr &conn)
{
    LOG_TRACE<<conn->localAddress().toIpPort()<<" -> "<<conn->peerAddress().toIpPort()
    <<" is "<<((conn->connected())?"UP":"DOWN");
}

void maya::net::defaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp receiveTime)
{
    buffer->retrieveAll();
}


TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd, const InetAddress localAddr,
                             const InetAddress perrAddr)
:loop_(CHECK_NOTNULL(loop)),
name_(name),
reading_(true),
socket_(new Socket(sockfd)),
channel_(new Channel(loop,sockfd)),
localAddr_(localAddr),
peerAddr_(perrAddr),
highWaterMark_(60*1024*1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this,_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleError, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG<<"TcpConnection::ctor["<< name_<<"] at"<< this<<" fd = "<<sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG<<"TcpConnection::dtor["<<name_<<"] at"<<this<<
    " fd = "<<channel_->fd()<<" state = "<<stateToString();
    assert(state_==kDisconnected);
}
bool TcpConnection::getTcpInfo(struct tcp_info * tcpi) const
{
    return socket_->getTcpinfo(tcpi);
}

string TcpConnection::getTcpInfoString() const
{
    char buf[1024];
    buf[0]='\0';
    socket_->getTcpinfoString(buf,sizeof(buf));
    return buf;
}

void TcpConnection::send(const void *message, size_t len)
{
    send(std::string(static_cast<const char*>(message),len));
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
            loop_->runInLoop(std::bind(static_cast<void(TcpConnection::*)(const string&)>(&TcpConnection::sendInLoop),
                                       this,message));
        }
    }
}

void TcpConnection::send(Buffer *message)
{
    if(state_==kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message->peek(),message->readableBytes());
            message->retrieveAll();
        }
        else
        {
            loop_->runInLoop(std::bind(static_cast<void (TcpConnection::*)(const string&)>(&TcpConnection::sendInLoop),
                                       this,message->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const string &message)
{
    sendInLoop(message.c_str(),message.size());
}
void TcpConnection::sendInLoop(const void *data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote=0;
    size_t remaining=len;
    bool faultError= false;
    if(state_==kDisconnected)
    {
        LOG_WARN<<"disconnected, give up writing";
        return;
    }
    if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0)
    {
        nwrote=::write(channel_->fd(),data,len);
        if(nwrote>=0)
        {
            remaining=len-nwrote;
            if(remaining==0 && writeCompleteCallback_)
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
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining<len);
    if(!faultError&&remaining>0)
    {
        size_t oldLen=outputBuffer_.readableBytes();
        if(oldLen+remaining>highWaterMark_
        &&oldLen<highWaterMark_
        &&highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(),oldLen+remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data)+nwrote,remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
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

void TcpConnection::forceClose()
{
    if(state_==kConnected||state_==kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop,shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

const char* TcpConnection::stateToString() const
{
    switch (state_)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
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