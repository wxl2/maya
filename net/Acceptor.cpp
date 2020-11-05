//
// Created by wxl on 2020/11/3.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include "../base/Logging.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace maya;
using namespace maya::net;
Acceptor::Acceptor(maya::net::EventLoop *loop, const maya::net::InetAddress &listenAddr)
:loop_(loop),
acceptSocket_(sockets::createNonbolckingOrDie()),
acceptChannel_(loop,acceptSocket_.fd()),
listenning_(false)
{
    acceptSocket_.setReuseaddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_= true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd=acceptSocket_.accept(peerAddr);
    if(connfd>=0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd,peerAddr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
}
