//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_TCPCLIENT_H
#define MAYA_TCPCLIENT_H
#include "../base/nocopyable.h"
#include "../base/Types.h"
#include "TcpConnection.h"
#include <memory>
#include <mutex>

namespace maya{
namespace net{

    class Connector;

    typedef std::shared_ptr<Connector> ConnectorPtr;

    class TcpClient:nocopyable
    {
     public:
        TcpClient(EventLoop* loop,const InetAddress& serverAddr,const string& nameArg);
        ~TcpClient();

        void connect();
        void disconnect();
        void stop();

        TcpConnectionPtr connection()const
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return connection_;
        }

        EventLoop* getLoop() const{return loop_;}

        bool retry()const;
        void enableRetry(){retry_= true;}

        const string& name() const{return name_;}

        void setConnectionCallback(const  ConnectionCallback& cb)
        {connectionCallback_=cb;}

        void setMessageCallback(const MessageCallback& cb)
        {messageCallback_=cb;}

        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {writeCompleteCallback_=cb;}

    private:
        void newConnection(int sockfd);
        void removeConnection(const TcpConnectionPtr& conn);

    private:
        EventLoop *loop_;
        ConnectorPtr connector_;
        const std::string name_;
        ConnectionCallback connectionCallback_;
        MessageCallback  messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool retry_;
        bool connect_;
        int nextConnId_;
        mutable std::mutex mutex_;
        TcpConnectionPtr connection_;
    };

}//namespace net
}//namespace maya



#endif //MAYA_TCPCLIENT_H
