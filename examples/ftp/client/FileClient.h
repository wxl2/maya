//
// Created by wxl on 2020/12/2.
//

#ifndef MAYA_FILECLIENT_H
#define MAYA_FILECLIENT_H

#include "net/EventLoop.h"
#include "net/TcpClient.h"
#include <mutex>
#include <condition_variable>

namespace maya{
namespace net{

    /*!
     * 客户端应该阻塞等待文件下载完成
     * 如果在传输过程中还需要再传输文件
     * 可以再开一个客户端
     */
    class FileClient:nocopyable
    {
    public:
        FileClient(EventLoop* loop,const InetAddress& address);
        void connect();
        void disconnect();

        void onRead(const std::shared_ptr<TcpConnection>& conn,Buffer* buffer,Timestamp receivTime);
        bool processLine(std::string& cmdline);
    private:
        void onConnection(const TcpConnectionPtr& conn);
        bool process(const std::shared_ptr<TcpConnection>& conn,const char* inbuf,size_t length);
        bool onDownloadFileRequest(const std::string& filename, int64_t offset, int64_t filesize,const std::string& filedata);
        bool onUploadFileRequest(const std::string& filename);
        bool onListFile(std::string& filedata,size_t length);

        bool onDownload(const std::string& filename);
        bool onUpload(const std::string& fielname);
        void resetFile();

        void send(int32_t cmd,int32_t seq,const std::string& filename,int64_t offset,int64_t filesize, const std::string& filedata);
        void sendPackage(const char* body,int64_t bodylength);
    private:
        int32_t                 seq_;
        FILE*                   fp_;
        TcpClient               client_;
        std::mutex              mutex_;
        std::condition_variable cond_;
        TcpConnectionPtr        conn_;
        int64_t                 currentDownloadFileOffset_;
        int64_t                 currentDownloadFileSize_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_FILECLIENT_H
