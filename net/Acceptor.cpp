//
// Created by wxl on 2020/11/3.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include "base/Logging.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace maya;
using namespace maya::net;
Acceptor::Acceptor(maya::net::EventLoop *loop, const maya::net::InetAddress &listenAddr, bool reuseport)
:loop_(loop),
acceptSocket_(sockets::createNonbolckingOrDie()),
acceptChannel_(loop,acceptSocket_.fd()),
listenning_(false),
idleFd_(::open("/dev/null",O_RDONLY|O_CLOEXEC))
{
    assert(idleFd_>=0);
    acceptSocket_.setReuseaddr(true);
    acceptSocket_.setReusePort(reuseport);
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
    InetAddress peerAddr;
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
    else
    {
        LOG_SYSERR<<"in Accecptor::handleRead";
        if(errno==EMFILE)//文件描述符打开达到系统上限
        {
            ///一个不好理解的变量是idleFd_;，
            /// 它是一个文件描述符，这里是打开"/dev/null"文件后返回的描述符，用于解决服务器端描述符耗尽的情况。
            ///如果当服务器文件描述符耗尽后，服务器端accept还没等从tcp连接队列中取出连接请求就已经失败返回了，
            /// 此时内核tcp队列中一直有客户端请求，内核会一直通知监听套接字，导致监听套接字一直处于可读，在下次直接poll函数时会直接返回。
            ///解决的办法就是在服务器刚启动时就预先占用一个文件描述符，通常可以是打开一个文件，这里是"/dev/null"。
            /// 此时服务器就有一个空闲的文件描述符了，当出现上述情况无法取得tcp连接队列中的请求时，先关闭这个文件让出一个文件描述符，
            /// 此时调用accept函数再次接收，由于已经有一个空闲的文件描述符了，accept会正常返回，将连接请求从tcp队列中取出，
            /// 然后优雅的关闭这个tcp连接（调用close函数），最后再打开"/dev/null"这个文件把”坑“占住。
            ::close(idleFd_);
            idleFd_=::accept(acceptSocket_.fd(),NULL,NULL);
            ::close(idleFd_);
            idleFd_=open("/dev/null",O_CLOEXEC|O_RDONLY);
        }
    }
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}
