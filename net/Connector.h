//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_CONNECTOR_H
#define MAYA_CONNECTOR_H

#include "../base/nocopyable.h"
#include "../base/Types.h"
#include "InetAddress.h"
#include "Channel.h"
#include <functional>
#include <memory>

namespace maya{
namespace net{

    class EventLoop;
    class InetAddress;

class Connector:nocopyable
, public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;
    Connector(EventLoop* loop,const InetAddress &serverAddr);
    ~Connector();

    void setNewConnectionCallback(const  NewConnectionCallback& cb){newConnectionCallback_=cb;}

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const{return servAddr_;}

private:
    enum States{kDisconnected,kConnecting,kConnected};
    static const int kMaxRetryDelayMs=30*1000;
    static const int kInitRetryDelayMs=500;

    void setStates(States s){states_=s;}
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    NewConnectionCallback  newConnectionCallback_;
    InetAddress servAddr_;
    States states_;
    bool connect_;
    std::unique_ptr<Channel> channel_;
    int retryDelayMs_;
};

}//namespace net
}//namespace maya

#endif //MAYA_CONNECTOR_H
