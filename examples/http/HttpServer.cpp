//
// Created by wxl on 2020/11/12.
//

#include "HttpServer.h"
#include "HttpRequest.h"
#include "base/Logging.h"
#include "HttpContext.h"
#include "HttpResponse.h"

using namespace maya;
using namespace maya::net;

namespace maya{
namespace net{
namespace detail{

        void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
        {
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }

}  // namespace detail
}  // namespace net
}  // namespace muduo

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const string& name,
                       TcpServer::Option option)
        : server_(loop, listenAddr, name, option),
          getCallback_(detail::defaultHttpCallback),
          postCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(
            std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(
            std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start()
{
    LOG_WARN << "HttpServer[" << server_.name()
             << "] starts listenning on " << server_.ipPort();
    if(initCallback_)
        initCallback_();
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        HttpContext context;
        std::lock_guard<std::mutex> lock(mutex_);
        connMaps_[conn]=context;
    }
    else
    {
        onDisconnected(conn);
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
    //HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());
    connMap::const_iterator it=connMaps_.end();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        it=connMaps_.find(conn);
    }
    assert(it!=connMaps_.end());
    HttpContext context =it->second;
    if (!context.parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context.gotAll())
    {
        onRequest(conn, context.request());
        context.reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    if(req.method()==HttpRequest::kGet)
    {
        doGet(req,&response);
    }
    else if(req.method()==HttpRequest::kPost)
    {
        doPost(req,&response);
    }
    else
    {
        detail::defaultHttpCallback(req,&response);
    }
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

void HttpServer::onDisconnected(const TcpConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connMap::const_iterator it= connMaps_.find(conn);
    assert(it!=connMaps_.end());
    connMaps_.erase(it);
}

void HttpServer::doPost(const HttpRequest &req, HttpResponse *resp)
{
    postCallback_(req,resp);
}

void HttpServer::doGet(const HttpRequest &req, HttpResponse *resp)
{
    getCallback_(req,resp);
}

