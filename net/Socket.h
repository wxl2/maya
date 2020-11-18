//
// Created by wxl on 2020/11/3.
//

#ifndef MAYA_SOCKET_H
#define MAYA_SOCKET_H

#include "base/nocopyable.h"

struct tcp_info;
namespace maya{
namespace net{

    class InetAddress;
    class Socket:nocopyable
    {
    public:
        explicit Socket(int sockfd)
        :sockfd_(sockfd)
        { }

        ~Socket();

        int fd() const{return sockfd_;}

        bool getTcpinfo(struct tcp_info*) const;
        bool getTcpinfoString(char* buf,int len) const;

        void bindAddress(const InetAddress& localaddr);
        void listen();

        int accept(InetAddress& peeraddr);
        void shutdownWrite();

        //开启禁用Nagle算法
        void setTcpNoDelay(bool on);

        //端口复用SO_REUSEADDR
        void setReuseaddr(bool on);

        //开启多个进程绑定一个端口
        void setReusePort(bool on);

        void setKeepAlive(bool on);
    private:
        const int sockfd_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_SOCKET_H
