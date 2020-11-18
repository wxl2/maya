//
// Created by wxl on 2020/11/5.
//

#include "base/Logging.h"
#include "SocketsOps.h"
#include "Connector.h"
#include "EventLoop.h"

using namespace maya;
using namespace maya::net;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
:loop_(loop),
servAddr_(serverAddr),
connect_(false),
states_(kDisconnected),
retryDelayMs_(kInitRetryDelayMs)
{
    LOG_DEBUG<<"ctor["<<this<<"]";
}

Connector::~Connector()
{
    LOG_DEBUG<<"dtor["<< this<<"]";
    assert(!channel_);
}

void Connector::start()
{
    connect_= true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::stop()
{
    connect_= false;
    loop_->runInLoop(std::bind(&Connector::stopInLoop,this));
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(states_==kDisconnected);
    if(connect_)
    {
        connect();
    }
    else
    {
        LOG_DEBUG<<"do not connect";
    }
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if(states_==kConnecting)
    {
        setStates(kDisconnected);
        int sockfd=removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setStates(kDisconnected);
    retryDelayMs_=kInitRetryDelayMs;
    connect_= true;
    stopInLoop();
}

void Connector::connect()
{
    int sockfd = sockets::createNonbolckingOrDie();
    int ret=sockets::connect(sockfd,&servAddr_.getSockAddrInet());
    int saveErrno=(ret==0)?0:errno;
    switch (saveErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;
        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_SYSERR<<"connect error in Connection::startInLoop"<<saveErrno;
            sockets::close(sockfd);
            break;
        default:
            LOG_SYSERR<<"Unexpected error in Connection::startInLoop"<<saveErrno;
            sockets::close(sockfd);
            break;
    }
}

void Connector::connecting(int sockfd)
{
    setStates(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_,sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError,this));

    channel_->enableWriting();
}


int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd=channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    LOG_TRACE<<"Connector::handleWrite"<<states_;
    if(states_==kConnecting)
    {
        int sockfd=removeAndResetChannel();
        int err=sockets::getSocketError(sockfd);
        if(err)
        {
            LOG_WARN<<"Connector::handleWrite -SO_ERROR = "<<err<<" "<<strerror_tl(err);
            retry(sockfd);
        }
        else
        {
            setStates(kConnected);
            if(connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        assert(states_==kDisconnected);
    }
}

void Connector::handleError()
{
    LOG_ERROR<<"Connection::handleError state = "<<states_;
    if(states_==kConnected)
    {
        int sockfd=removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_TRACE<<"SO_ERROR = "<<err<<strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setStates(kDisconnected);
    if(connect_)
    {
        LOG_INFO<<"Connector::retry Retry connecting to "<<servAddr_.toIpPort()
        <<" in "<<retryDelayMs_<<" milliseconds";
        loop_->runAfter(retryDelayMs_/1000.0,std::bind(&Connector::startInLoop,shared_from_this()));
        retryDelayMs_=std::min(retryDelayMs_*2,kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG<<"do not connect";
    }
}


