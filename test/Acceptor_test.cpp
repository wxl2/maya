//
// Created by wxl on 2020/11/4.
//

#include "../net/Acceptor.h"
#include "../net/SocketsOps.h"
#include "../net/InetAddress.h"
#include "Endian.h"
#include "../net/EventLoop.h"
#include <unistd.h>
using namespace maya;
using namespace maya::net;
void newConnection(int sockfd,const maya::net::InetAddress& peeraddr)
{
    printf("newConnection() accepted a new connection from %s\n",peeraddr.toIpPort().c_str());
    ::write(sockfd,"OK\n",4);
    maya::net::sockets::close(sockfd);
}

int main()
{
    printf("main() : pid = %d\n",getpid());
    maya::net::InetAddress addr(9999);
    maya::net::EventLoop loop;

    maya::net::Acceptor acceptor(&loop,addr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();
    loop.loop();
}


