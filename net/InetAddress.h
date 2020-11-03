//
// Created by wxl on 2020/10/30.
//

#ifndef MAYA_INETADDRESS_H
#define MAYA_INETADDRESS_H
#include "../base/copyable.h"
#include <arpa/inet.h>
#include "../base/Types.h"
namespace maya{
namespace net{
namespace sockets{
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
}//namespace sockets

    class InetAddress:copyable
    {
    public:
        explicit InetAddress(uint16_t port=0,bool loopbackOnly= false);
        InetAddress(const std::string& ip,uint16_t port);
        InetAddress(const struct sockaddr_in& addr)
            :addr_(addr)
            { }
        string toIp() const;
        string toIpPort() const;
        uint16_t toPort() const;

        const struct sockaddr_in& getSockAddrInet() const{return addr_;}
        void setSockAddrInet(const struct sockaddr_in& addr){addr_=addr;}

        uint32_t ipNetEndian() const{return addr_.sin_addr.s_addr;}//返回ip字节序
        uint16_t portNetEndian ()const {return addr_.sin_port;}//返回端口字节序

        static bool reslove(const string& hostname,InetAddress* result);//使用域名初始化
    private:
        struct sockaddr_in addr_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_INETADDRESS_H
