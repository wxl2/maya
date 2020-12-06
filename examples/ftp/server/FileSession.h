//
// Created by wxl on 2020/11/30.
//

#ifndef MAYA_FILESESSION_H
#define MAYA_FILESESSION_H

#include "TcpSession.h"
#include "net/Buffer.h"

namespace maya{
namespace net{

    //一个fileseeion对应一次客户端会话,在连接没有关闭之前都是存活的
    class FileSession: public TcpSession//继承于TcpSeeion,将业务与逻辑分开
    {
    public:
        FileSession(const std::shared_ptr<TcpConnection>& conn,const char* filebasedir);
        virtual ~FileSession();

        void onRead(const std::shared_ptr<TcpConnection>& conn,Buffer* buffer,Timestamp receivTime);

    private:
        bool process(const std::shared_ptr<TcpConnection>& conn,const char* inbuf,size_t length);
        bool onUploadFileResponse(const std::string& filename, int64_t offset, int64_t filesize,
                                  const std::string& filedata, const std::shared_ptr<TcpConnection>& conn);
        bool onDownloadFileResponse(const std::string& filename,const std::shared_ptr<TcpConnection>& conn);
        bool onListFile(const std::shared_ptr<TcpConnection>& conn,size_t length,std::string& filedata);

        void resetFile();
    private:
        static int32_t           id_; //session id
        int32_t           seq_;//当前session数据包序列号

        //当前会话信息
        FILE*             fp_;
        int64_t           currentDownloadFileOffset_;      //当前在正下载的文件的偏移量
        int64_t           currentDownloadFileSize_;        //当前在正下载的文件的大小(下载完成以后最好置0)
        std::string       strFileBaseDir_;                 //文件最基础目录
        std::string       strFileCurrentDir_;              //当前会话所在的文件目录
        bool              bFileUploading_;                 //是否处于正在上传文件的过程中
    };
}//namespace net
}//namespace maya


#endif //MAYA_FILESESSION_H
