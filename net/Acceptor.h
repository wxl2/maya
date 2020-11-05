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
        Acceptor(EventLoop* loop,const InetAddress& listenAddr);
        void setNewConnectionCallback(const NewConnectionCallback& cb)
        {newConnectionCallback_=cb;}

        bool listenning() const{return listenning_;}
        void listen();
    private:
        void handleRead();
        EventLoop* loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listenning_;
    };
}
}


#endif //MAYA_ACCEPTOR_H
