//
// Created by wxl on 2020/11/12.
//

#ifndef MAYA_HTTPSERVER_H
#define MAYA_HTTPSERVER_H

#include "net/TcpServer.h"
#include "HttpContext.h"
#include <mutex>

namespace maya{
namespace net{

    class HttpRequest;
    class HttpResponse;
    class HttpServer:nocopyable
    {
    public:
        typedef std::function<void (const HttpRequest&,HttpResponse*)> HttpCallback;
        typedef std::function<void ()> InitCallback;
        HttpServer(EventLoop* loop,const InetAddress& listenAddr,
                   const string& name,
                   TcpServer::Option option=TcpServer::kNoReusePort);

        EventLoop* getLoop()const{return server_.getLoop();}
        /// Not thread safe, callback be registered before calling start().//在调用start函数后调用
        void setGetCallback(const HttpCallback& cb)
        {
            getCallback_ = cb;
        }

        void setPostCallback(const HttpCallback& cb)
        {
            postCallback_ = cb;
        }

        void setInitCallback(const InitCallback& cb)
        {
            initCallback_ = cb;
        }

        void setThreadNum(int numThreads)
        {
            server_.setThreadNum(numThreads);
        }

        void start();

    private:
        typedef std::map<TcpConnectionPtr,HttpContext> connMap;
        void onConnection(const TcpConnectionPtr& conn);
        void onDisconnected(const  TcpConnectionPtr& conn);
        void onMessage(const TcpConnectionPtr& conn,Buffer* buf,Timestamp receviceTime);
        void onRequest(const TcpConnectionPtr& conn,const HttpRequest&);
        void doPost(const HttpRequest& req, HttpResponse* resp);
        void doGet(const HttpRequest& req, HttpResponse* resp);

        TcpServer server_;
        HttpCallback getCallback_;
        InitCallback initCallback_;
        HttpCallback postCallback_;
        connMap connMaps_;
        std::mutex mutex_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_HTTPSERVER_H
