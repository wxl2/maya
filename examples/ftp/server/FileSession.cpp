//
// Created by wxl on 2020/11/30.
//

#include "FileSession.h"
#include "FileMsg.h"
#include "FileManager.h"
#include "base/Logging.h"
#include "utils/ProtocolStream.h"
#include "base/Singleton.h"

#define MAX_PACKGE_SIZE 50*1024*1024
using namespace maya;
using namespace maya::net;

FileSession::FileSession(const std::shared_ptr<TcpConnection> &conn, const char *filebasedir)
:TcpSession(conn),
m_id(0),m_seq(0),m_fp(NULL),
m_currentDownloadFileOffset(0),
m_currentDownloadFileSize(0),
m_strFileBaseDir(filebasedir),
m_bFileUploading(false)
{
}

FileSession::~FileSession()
{
}

void FileSession::resetFile()
{
    if(m_fp!=NULL)
    {
        fclose(m_fp);
        m_fp=NULL;
        m_currentDownloadFileSize=0;
        m_currentDownloadFileOffset=0;
        m_bFileUploading= false;
    }
}

void FileSession::onRead(const std::shared_ptr<TcpConnection> &conn, Buffer *buffer, Timestamp receivTime)
{
    while (true)
    {
        //不够一个包头大小
        if(buffer->readableBytes()<(size_t)(sizeof(file_msg_header)))
        {
            LOG_INFO << "buffer is not enough for a package header, pBuffer->readableBytes()=" << buffer->readableBytes() << ", sizeof(msg)=" << sizeof(file_msg_header);
            return;
        }

        file_msg_header header;
        memcpy(&header,buffer->peek(),sizeof(header));

        //非法数据包,关闭连接
        if(header.packagesize<=0||header.packagesize>=MAX_PACKGE_SIZE)
        {
            //客户端发非法数据包，服务器主动关闭之
            LOG_ERROR<<"Illegal package header size: "<<header.packagesize<<" close TcpConnection, client: "<< conn->peerAddress().toIpPort().c_str();
            conn->forceClose();
            return;
        }

        //不够一个整包大小
        if(buffer->readableBytes()<(size_t)header.packagesize+sizeof(file_msg_header))
            return;
        buffer->retrieve(sizeof(file_msg_header));
        std::string inbuf;
        inbuf.append(buffer->peek(),header.packagesize);
        buffer->retrieve(header.packagesize);
        if(!process(conn,inbuf.c_str(),inbuf.size()))
        {
            LOG_ERROR<<"Process error, close TcpConnection, client: "<<conn->peerAddress().toIpPort().c_str();
            conn->forceClose();
        }
    }//end of while
}

bool FileSession::process(const std::shared_ptr<TcpConnection> &conn, const char *inbuf, size_t length)
{
    maya::detail::BinaryStreamReader readStream(inbuf,length);
    int32_t cmd;
    if(!readStream.ReadInt32(cmd))
    {
        LOG_ERROR<<"read cmd error, client "<<conn->peerAddress().toIpPort();
        return false;
    }
    if(!readStream.ReadInt32(m_seq))
    {
        LOG_ERROR<<"read seq error, client "<<conn->peerAddress().toIpPort();
        return false;
    }

    std::string filename;
    size_t filenameLength;
    if(!readStream.ReadString(&filename,0,filenameLength)||filenameLength==0)
    {
        LOG_ERROR<<"read file error, client "<<conn->peerAddress().toIpPort();
        return false;
    }

    int64_t offset;
    if(!readStream.ReadInt64(offset))
    {
        LOG_ERROR<<"read offset error, client "<<conn->peerAddress().toIpPort();
        return false;
    }
    int64_t filesize;
    if(!readStream.ReadInt64(filesize))
    {
        LOG_ERROR<<"read filesize error, client"<<conn->peerAddress().toIpPort();
        return false;
    }
    std::string filedata;
    size_t filedataLength;
    if(!readStream.ReadString(&filedata,0,filedataLength))
    {
        LOG_ERROR<<"read filedata error, client"<<conn->peerAddress().toIpPort();
        return false;
    }

    LOG_INFO<<"Rquest from client: cmd: "<<cmd<<" filename: "<<filename<<" fileLength: "<<filenameLength
    <<" offset: "<<offset<<" filesize: "<<filesize<<" filedataLength: "<<filedataLength
    <<"header.packagesize: "<<length<<" client: "<<conn->peerAddress().toIpPort();

    switch (cmd)
    {
        case msg_type_upload_req:
            return onUploadFileResponse(filename,offset,filesize,filedata,conn);
        case msg_type_download_req:
        {
            int32_t clientNetType;
            if(!readStream.ReadInt32(clientNetType))
            {
                LOG_ERROR<<"read clientNetTpye error, client: "<<conn->peerAddress().toIpPort();
                return false;
            }
            return onDownloadFileResponse(filename,clientNetType,conn);
        }
        default:
            LOG_ERROR<<"unknown cmd, cmd: "<<cmd<<" client: "<<conn->peerAddress().toIpPort();
            return false;
    }
}

bool FileSession::onUploadFileResponse(const string &filename, int64_t offset, int64_t filesize, const string &filedata,
                                       const std::shared_ptr<TcpConnection> &conn)
{
    if(filename.empty())
    {
        LOG_ERROR<<"Empty filename, client: "<<conn->peerAddress().toIpPort();
        return false;
    }
    if(Singleton<FileManager>::instance().isFileExist(filename.c_str())&&m_bFileUploading)
    {
        offset=filesize;
        string dummyfiledata;
        send(msg_type_upload_resp,m_seq,file_msg_error_complete,filename,offset,filesize,dummyfiledata);

        return true;
    }
}

bool FileSession::onDownloadFileResponse(const string &filename, int32_t clientNetType,
                                     const std::shared_ptr<TcpConnection> &conn)
{
    return false;
}
