//
// Created by wxl on 2020/10/30.
//

#include "InetAddress.h"
#include "Endian.h"
#include "SocketsOps.h"
#include "base/Logging.h"
#include <netdb.h>

using namespace maya;
using namespace maya::net;

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

InetAddress::InetAddress(uint16_t port,bool loopbackOnly)
{
   memZero(&addr_,sizeof(addr_));
   addr_.sin_family=AF_INET;
   in_addr_t ip=loopbackOnly?kInaddrLoopback:kInaddrAny;
   addr_.sin_addr.s_addr=sockets::hostToNetwork32(ip);
   addr_.sin_port=sockets::hostToNetwoek16(port);
}

InetAddress::InetAddress(const string &ip, uint16_t port)
{
    memZero(&addr_,sizeof(addr_));
    sockets::fromIpPort(ip.c_str(),port,&addr_);
}

string InetAddress::toIp() const
{
    char buf[32];
    sockets::toIp(buf,sizeof(buf),&addr_);
    return buf;
}

string InetAddress::toIpPort() const
{
    char buf[32];
    sockets::toIpPort(buf,sizeof(buf),&addr_);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return sockets::networkToHost16(addr_.sin_port);
}

static thread_local char t_resloveBuffer[64*1024];
bool InetAddress::reslove(const string &hostname, InetAddress *result)
{
    assert(result!=NULL);
    struct hostent hent;
    struct hostent* he=NULL;
    int herrno=0;
    memZero(&hent,sizeof(hent));
    int ret = gethostbyname_r(hostname.c_str(),&hent,t_resloveBuffer,sizeof(t_resloveBuffer),&he,&herrno);
    if(ret==0 &&he!=NULL)
    {
        assert(he->h_addrtype==AF_INET&&he->h_length==sizeof(uint32_t));
        result->addr_.sin_addr=*reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if(ret)
        {
            LOG_SYSERR<<"InetAddress::reslove";
        }
        return false;
    }
}
