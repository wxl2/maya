//
// Created by wxl on 2020/11/30.
//

#ifndef MAYA_FILESESSION_H
#define MAYA_FILESESSION_H

#include "TcpSession.h"
#include "net/Buffer.h"

namespace maya{
namespace net{

    class FileSession: public TcpSession
    {
    public:
        FileSession(const std::shared_ptr<TcpConnection>& conn,const char* filebasedir);
        virtual ~FileSession();

        void onRead(const std::shared_ptr<TcpConnection>& conn,Buffer* buffer,Timestamp receivTime);

    private:
        bool process(const std::shared_ptr<TcpConnection>& conn,const char* inbuf,size_t length);
        bool onUploadFileResponse(const std::string& filename, int64_t offset, int64_t filesize,
                                  const std::string& filedata, const std::shared_ptr<TcpConnection>& conn);
        bool onDownloadFileResponse(const std::string& filename, int32_t clientNetType/*客户端网络类型*/,
                                    const std::shared_ptr<TcpConnection>& conn);

        void resetFile();
    private:
        int32_t           m_id; //session id
        int32_t           m_seq;//当前session数据包序列号

        //当前文件信息
        FILE*             m_fp;
        int64_t           m_currentDownloadFileOffset;      //当前在正下载的文件的偏移量
        int64_t           m_currentDownloadFileSize;        //当前在正下载的文件的大小(下载完成以后最好置0)
        std::string       m_strFileBaseDir;                 //文件目录
        bool              m_bFileUploading;                 //是否处于正在上传文件的过程中
    };
}//namespace net
}//namespace maya


#endif //MAYA_FILESESSION_H
