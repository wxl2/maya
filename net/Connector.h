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

    //连接器,负责发起连接,不负责创建socket
class Connector:nocopyable
, public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;
    Connector(EventLoop* loop,const InetAddress &serverAddr);
    ~Connector();

    void setNewConnectionCallback(const  NewConnectionCallback& cb){newConnectionCallback_=cb;}

    //can be called in any thread
    void start();
    //must be called in loop thread
    void restart();
    //can be called in any thread
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
    //重试,重新连接
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    NewConnectionCallback  newConnectionCallback_;
    InetAddress servAddr_;
    States states_;
    bool connect_;
    //将其中保存的channel注册到IO线程的监听列表中,而后继续利用这个指针来发起连接并注册到IO线程中
    std::unique_ptr<Channel> channel_;
    int retryDelayMs_;
};

}//namespace net
}//namespace maya

#endif //MAYA_CONNECTOR_H
