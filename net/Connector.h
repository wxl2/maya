//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_CONNECTOR_H
#define MAYA_CONNECTOR_H

#include "../base/nocopyable.h"
#include "../base/Types.h"
#include <functional>
#include <memory>

namespace maya{
namespace net{

    class EventLoop;
    class InetAddress;

class Connector:nocopyable
{
public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;
    Connector(EventLoop* loop,const InetAddress &serverAddr);
    ~Connector();

    void setNewConnectionCallback(const  NewConnectionCallback& cb){newConnectionCallback_=cb;}

    void start();
    void restart();
    void stop();

private:
    NewConnectionCallback  newConnectionCallback_;
};

}//namespace net
}//namespace maya

#endif //MAYA_CONNECTOR_H
