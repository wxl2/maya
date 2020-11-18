//
// Created by wxl on 2020/10/30.
//

#include "SocketsOps.h"
#include "base/Logging.h"
#include "Endian.h"

#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>

using namespace maya;
using namespace maya::net;

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
   return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr *addr)
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

struct sockaddr_in* sockets::sockaddr_in_cast(struct sockaddr *addr)
{
    return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}

int sockets::createNonbolckingOrDie()
{
    int sockfd=::socket(AF_INET,SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK,IPPROTO_TCP);
    if(sockfd<0)
    {
        LOG_SYSFATAL<<"sockets::createNonblockOrDie";
    }
    return  sockfd;
}

int sockets::connect(int sockfd,const struct sockaddr_in* addr)
{
    return ::connect(sockfd,sockaddr_cast(addr),static_cast<socklen_t>(sizeof(*addr)));
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in* addr)
{
    int ret=::bind(sockfd,sockaddr_cast(addr),static_cast<socklen_t>(sizeof(*addr)));
    if(ret<0)
    {
        LOG_SYSFATAL<<"sockets::bindOrDie";
    }
}


void sockets::listenOrDie(int sockfd)
{
    int ret=::listen(sockfd,SOMAXCONN);
    if(ret<0)
    {
        LOG_SYSFATAL<<"sockets::listenOrDie";
    }
}


int  sockets::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen=static_cast<socklen_t>(sizeof(*addr));

    int connfd=::accept4(sockfd,sockaddr_cast(addr),&addrlen,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd<0)
    {
        int savedErrno=errno;
        LOG_SYSERR<<"sockets::accept";
        switch (savedErrno)
        {
            case EACCES:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno=savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
}

ssize_t sockets::read(int sockfd,void* buf,size_t count)
{
    return ::read(sockfd,buf,count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd,iov,iovcnt);
}

size_t sockets::write(int sockfd,void* buf,size_t count)
{
    return ::write(sockfd,buf,count);
}


void sockets::close(int sockfd)
{
    if(::close(sockfd)<0)
    {
        LOG_SYSERR<<"sockets::close";
    }
}

void sockets::shutdownWrite(int sockfd)
{
    if(::shutdown(sockfd,SHUT_WR)<0)
    {
        LOG_SYSERR<<"sockets::shutdownWrite";
    }
}

void sockets::toIpPort(char *buf, size_t size, const struct sockaddr_in *addr)
{
    ::inet_ntop(AF_INET,&addr->sin_addr,buf,static_cast<socklen_t>(size));
    size_t end=::strlen(buf);
    assert(size>end);
    uint16_t port=sockets::networkToHost16(addr->sin_port);
    snprintf(buf+end,size-end,":%u",port);
}

void sockets::toIp(char *buf,size_t size,const struct sockaddr_in* addr)
{
    assert(size>=sizeof(struct sockaddr_in));
    ::inet_ntop(AF_INET,&addr->sin_addr,buf,static_cast<socklen_t>(size));
}

void sockets::fromIpPort(const char* ip,uint16_t port,struct sockaddr_in* addr)
{
    addr->sin_family=AF_INET;
    addr->sin_port=hostToNetwoek16(port);
    if(::inet_pton(AF_INET,ip,&addr->sin_addr)<=0)
    {
        LOG_SYSERR<<"sockets::formIpPort";
    }
}

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen=static_cast<socklen_t>(sizeof(optval));
    if(::getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&optval,&optlen)<0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}


struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    memZero(&localaddr,sizeof(localaddr));
    socklen_t addrlen=static_cast<socklen_t>(sizeof(localaddr));
    if(::getsockname(sockfd,sockaddr_cast(&localaddr),&addrlen))
    {
        LOG_SYSERR<<"sockets::getLocalAddr";
    }
    return localaddr;
}
struct sockaddr_in sockets::getPeerAddr(int sockfd)
{

    struct sockaddr_in peeraddr;
    memZero(&peeraddr,sizeof(peeraddr));
    socklen_t addrlen=static_cast<socklen_t>(sizeof(peeraddr));
    if(::getpeername(sockfd,sockaddr_cast(&peeraddr),&addrlen))
    {
        LOG_SYSERR << "sockets::getPeerAddr";
    }
    return peeraddr;
}

bool sockets::isSelfConnect(int sockfd)
{
    struct sockaddr_in localaddr=getLocalAddr(sockfd);
    struct sockaddr_in peeraddr=getPeerAddr(sockfd);
    return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}
