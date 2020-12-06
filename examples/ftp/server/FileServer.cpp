//
// Created by wxl on 2020/12/1.
//

#include "FileServer.h"

using namespace maya;
using namespace maya::net;


bool FileServer::init(const char *ip, short port, EventLoop *loop, const char *fileBasedir)
{

    fileBaseDir_=fileBasedir;
    InetAddress address(ip,port);
    server_.reset(new TcpServer(loop,address,"ftpserver",TcpServer::kReusePort));
    server_->setConnectionCallback(
            std::bind(&FileServer::onConnected, this, _1));
    server_->setThreadNum(0);
    LOG_WARN << "ftpServer[" << server_->name()
             << "] starts listenning on " << server_->ipPort();
    server_->start();
    return true;
}


void FileServer::onConnected(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        LOG_INFO<<"client connected: "<<conn->peerAddress().toIpPort();
        std::shared_ptr<FileSession> spSessio(new FileSession(conn,fileBaseDir_.c_str()));
        conn->setMessageCallback(std::bind(&FileSession::onRead,spSessio.get(),_1,_2,_3));

        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.push_back(spSessio);
    }
    else
    {
        onDisconnected(conn);
    }
}

void FileServer::onDisconnected(const TcpConnectionPtr& conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for(auto iter=sessions_.begin();iter!=sessions_.end();++iter)
    {
        if((*iter)->getConnectionPtr()==NULL)
        {
            LOG_ERROR<<"connected is NULL";
            break;
        }
        sessions_.erase(iter);
        LOG_INFO<<"client disconnected: "<<conn->peerAddress().toIpPort();
        break;
    }
}

