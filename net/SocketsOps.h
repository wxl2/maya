//
// Created by wxl on 2020/10/30.
//

#ifndef MAYA_SOCKETSOPS_H
#define MAYA_SOCKETSOPS_H

#include <arpa/inet.h>

namespace maya{
namespace net{
namespace sockets{

    int createNonbolckingOrDie();
    int connect(int sockfd,const struct sockaddr_in* addr);
    void bindOrDie(int sockfd, const struct sockaddr_in* addr);
    void listenOrDie(int sockfd);
    int  accept(int sockfd, struct sockaddr_in* addr);
    ssize_t read(int sockfd,void* buf,size_t count);
    ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
    size_t write(int sockfd,void* buf,size_t count);
    void close(int sockfd);
    void shutdownWrite(int sockfd);
    void toIpPort(char* buf,size_t size,const struct sockaddr_in* addr);
    void toIp(char *buf,size_t size,const struct sockaddr_in* addr);
    void fromIpPort(const char* ip,uint16_t port,struct sockaddr_in* addr);
    int getSocketError(int sockfd);

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
    struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);

    const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr *addr);
    struct sockaddr_in* sockaddr_in_cast(struct sockaddr *addr);

    struct sockaddr_in getLocalAddr(int sockfd);//获取本机ip地址
    struct sockaddr_in getPeerAddr(int sockfd);//获取sockfd对端ip
    bool isSelfConnect(int sockfd);//判断是否是自己连接自己
}//namespace sockets
}//namespace net
}//namespace maya


#endif //MAYA_SOCKETSOPS_H
