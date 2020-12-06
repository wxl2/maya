//
// Created by wxl on 2020/12/1.
//

#ifndef MAYA_FILESERVER_H
#define MAYA_FILESERVER_H

#include "base/nocopyable.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"
#include "FileSession.h"
#include <memory>
#include <list>
#include <map>
#include <mutex>

namespace maya{
namespace net{

    class FileServer final :nocopyable
    {
    public:
        FileServer()=default;
        ~FileServer()=default;

        bool init(const char* ip,short port,EventLoop* loop,const char* fileBasedir="./file");

    private:
        //连接到来
        void onConnected(const TcpConnectionPtr& conn);
        //连接断开
        void onDisconnected(const TcpConnectionPtr& conn);
    private:
        std::unique_ptr<TcpServer>                  server_;
        std::list<std::shared_ptr<FileSession>>     sessions_;
        std::mutex                                  mutex_;
        std::string                                 fileBaseDir_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_FILESERVER_H
