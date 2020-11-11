//
// Created by wxl on 2020/11/3.
//

#ifndef MAYA_ACCEPTOR_H
#define MAYA_ACCEPTOR_H

#include "nocopyable.h"
#include "Channel.h"
#include "Socket.h"
#include <functional>

namespace maya{
namespace net{

    class Acceptor:nocopyable
    {
    public:
        typedef std::function<void(int sockfd,const InetAddress&)> NewConnectionCallback;
        Acceptor(EventLoop* loop,const InetAddress& listenAddr, bool reuseport);
        ~Acceptor();
        void setNewConnectionCallback(const NewConnectionCallback& cb)
        {newConnectionCallback_=cb;}

        bool listenning() const{return listenning_;}
        void listen();
    private:
        void handleRead();
        EventLoop* loop_;
        Socket acceptSocket_;//监听套接字文件描述符
        Channel acceptChannel_;//管理监听套接字
        NewConnectionCallback newConnectionCallback_;
        bool listenning_;
        int idleFd_;//处理文件描述符耗尽
    };
}
}


#endif //MAYA_ACCEPTOR_H
