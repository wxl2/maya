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

int32_t FileSession::id_=0;

FileSession::FileSession(const std::shared_ptr<TcpConnection> &conn, const char *filebasedir)
:TcpSession(conn),
seq_(0),
strFileBaseDir_(filebasedir),
strFileCurrentDir_(filebasedir),
currentDownloadFileOffset_(0),
currentDownloadFileSize_(0),
bFileUploading_(false)
{
    id_++;
}

FileSession::~FileSession()
{
}

void FileSession::onRead(const std::shared_ptr<TcpConnection> &conn, Buffer *buffer, Timestamp receivTime)
{
    while(true)
    {
        //不够一个包头大小
        if(buffer->readableBytes()<sizeof (file_msg_header))
        {
            LOG_INFO<<"buffer is not enough a packge header size, buffer->readableBytes() "<<buffer->readableBytes()
                    <<", sizeof(file_msg_header): "<<sizeof (file_msg_header)<<", client: "<<conn->peerAddress().toIpPort();
            return;
        }
        //不是一个合法数据包
        file_msg_header header;
        memcpy(&header,buffer->peek(),sizeof (file_msg_header));

        if(header.packagesize <=0||header.packagesize>MAX_PACKGE_SIZE)
        {
            LOG_ERROR<<"Illegal package header: size: "<<header.packagesize<<" close the TcpConnection, client :"
            <<conn->peerAddress().toIpPort();
            conn->forceClose();
            return;
        }

        if(buffer->readableBytes()<(size_t)header.packagesize+ sizeof(file_msg_header))
            return;

        buffer->retrieve(sizeof (file_msg_header));
        std::string buf;
        buf.append(buffer->peek(),header.packagesize);
        buffer->retrieve(header.packagesize);
        if(!process(conn,buf.c_str(),buf.size()))
        {
            LOG_ERROR<<"Perocess error, close TcpConnection: "<<conn->peerAddress().toIpPort();
            conn->forceClose();
        }
    }
}

bool FileSession::process(const std::shared_ptr<TcpConnection> &conn, const char *inbuf, size_t length)
{
    detail::BinaryStreamReader readerSteam(inbuf,length);
    int32_t cmd;
    ///命令头,文件包序列号,文件名,当前传输数据包在文件中的偏移量,文件大小,文件内容
    if(!readerSteam.ReadInt32(cmd))
    {
        LOG_ERROR<<"Read cmd error, client: "<<conn->peerAddress().toIpPort();
        return false;
    }
    if(!readerSteam.ReadInt32(seq_))
    {
        LOG_ERROR<<"Read seq error, client: "<<conn->peerAddress().toIpPort();
        return false;
    }
    std::string filename;
    size_t filenameLength;
    if(!readerSteam.ReadString(&filename,0,filenameLength)||filenameLength==0)
    {
        LOG_ERROR<<"Read filename error, client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    int64_t offset;
    if(!readerSteam.ReadInt64(offset))
    {
        LOG_ERROR<<"Read offset error, client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    int64_t filesize;
    if(!readerSteam.ReadInt64(filesize))
    {
        LOG_ERROR<<"Read filesize error, client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    std::string filedata;
    size_t filedataLength;
    if(!readerSteam.ReadString(&filedata,0,filedataLength))
    {
        LOG_ERROR<<"Read filedata error, client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    LOG_INFO<<"Request from client: "<<conn->peerAddress().toIpPort()<<" ,cmd: "<<cmd
    <<" ,seq: "<<seq_<<" ,filename: "<<filename<<" ,offset: "<<offset<<" ,filedatalength: "<<filedataLength;

    switch (cmd)
    {
        case msg_type_list_req:
            return onListFile(conn,filedataLength,filedata);
        case msg_type_download_req:
            return onDownloadFileResponse(filename,conn);
        case msg_type_upload_req:
            return onUploadFileResponse(filename,offset,filesize,filedata,conn);
        default:
            LOG_ERROR<<"unknown cmd type, client: "<<conn->peerAddress().toIpPort();
            return false;
    }
}

bool FileSession::onListFile(const std::shared_ptr<TcpConnection> &conn,size_t length,std::string& filedata)
{
    //server当前list的filedata为无效,可以直接忽略
    std::string buf;
    for(const auto&it:Singleton<FileManager>::instance().getFileList())
        buf.append(it+',');
    send(msg_type_list_resp,seq_++,file_msg_error_complete,"get",0,0,buf);
    return true;
}

bool FileSession::onDownloadFileResponse(const string &filename,const std::shared_ptr<TcpConnection> &conn)
 {
    if(filename.empty())
    {
        LOG_ERROR<<"Empty filename, client: "<<conn->peerAddress().toIpPort();
        return false;
    }
    if(!Singleton<FileManager>::instance().isFileExist(filename.c_str()))
    {
        std::string dummyfiledata;
        int64_t notExistFileOffset=0;
        int64_t notExistFileSize=0;
        send(msg_type_download_resp,seq_++,file_msg_error_not_exist,filename,notExistFileOffset,notExistFileSize,dummyfiledata);
        LOG_ERROR<<"filename not exist, filename: "<<filename<<" ,client: "<<conn->peerAddress().toIpPort();

        LOG_INFO<<"Response to client: cmd=msg_type_download_resp, errorcode=file_msg_error_not_exist"
        <<", filename: "<<filename<<" offeset: 0,filesize: 0, filedataLength: 0, client: "<<conn->peerAddress().toIpPort();
        return true;
        //这里并不断开连接
    }

    if(fp_==NULL)
    {
        std::string filenameLocal = strFileBaseDir_;
        filenameLocal += filename;
        fp_ = fopen(filenameLocal.c_str(), "rb+");
        if (fp_ == NULL) {
            LOG_ERROR << "fopen file error, filename: " << filename << " ,client: " << conn->peerAddress().toIpPort();
            return false;
        }
        if (fseek(fp_, 0, SEEK_END) == -1) {
            LOG_ERROR << "fseek file error, filename: " << filename << " ,client: " << conn->peerAddress().toIpPort();
            return false;
        }
        //获得文件大小
        currentDownloadFileSize_ = ftell(fp_);
        if (currentDownloadFileSize_ <= 0) {
            LOG_ERROR << "get filesize error, filesize: " << currentDownloadFileSize_ << ", client:"
                      << conn->peerAddress().toIpPort();
            return false;
        }
        //将文件移动只开头
        if (fseek(fp_, 0, SEEK_SET) == -1) {
            LOG_ERROR << "filesize seek start error, filesize: " << currentDownloadFileSize_ << ", client:"
                      << conn->peerAddress().toIpPort();
            return false;
        }
    }
    string filedata;
    int64_t currentSendSize=512*1024;
    char buf[512*1024]={0};
    if(currentDownloadFileSize_<=currentDownloadFileOffset_+currentSendSize)
    {
        currentSendSize=currentDownloadFileSize_-currentDownloadFileOffset_;
    }
    if(currentSendSize<=0||fread(buf,currentSendSize,1,fp_)!=1)
    {
        int saveErrno=errno;
        LOG_ERROR<<"fread error, filename: "<<filename<<" errno: "<<saveErrno<<" errorinfo: "
        <<strerror_tl(saveErrno)<<", currentSendSize: "<<currentSendSize<<" ,fp_: "<<fp_
        <<", buf size is "<<currentSendSize<<" ,client: "<<conn->peerAddress().toIpPort();
    }

    int sendOffset=currentDownloadFileOffset_;
    currentDownloadFileOffset_+=currentSendSize;
    filedata.append(buf,currentSendSize);
    int errorcode=file_msg_error_progress;
    if(currentDownloadFileOffset_==currentDownloadFileSize_)
        errorcode=file_msg_error_complete;

    send(msg_type_download_resp,seq_++,errorcode,filename,sendOffset,currentDownloadFileSize_,filedata);

    LOG_INFO<<"Response to client: cmd=msg_type_download_resp, errorcode="<< (errorcode == file_msg_error_progress ? "file_msg_error_progress" : "file_msg_error_complete")
    <<", filename: "<<filename<<"sendoffset: "<<sendOffset<<" filesize: "<<currentDownloadFileSize_<<" filedataLength"
    <<filedata.length()<<"download percent: "<<(currentDownloadFileSize_ * 100 / currentDownloadFileSize_) << "%"
    << ", client:" << conn->peerAddress().toIpPort();
    if(errorcode==file_msg_error_complete)
        resetFile();
    return true;
}

void FileSession::resetFile()
{
    if (fp_ != NULL)
    {
        fclose(fp_);
        fp_=NULL;
        currentDownloadFileSize_=0;
        currentDownloadFileOffset_=0;
        bFileUploading_ = false;
    }
}

bool FileSession::onUploadFileResponse(const string &filename, int64_t offset, int64_t filesize, const string &filedata,
                                       const std::shared_ptr<TcpConnection> &conn)
{
    if(filename.empty())
    {
        LOG_ERROR<<"Empty filename, client: "<<conn->peerAddress().toIpPort();
        return false;//非法数据包,关闭链接
    }
    if(Singleton<FileManager>::instance().isFileExist(filename.c_str())&&!bFileUploading_)
    {
        //文件已存在
        offset=filesize;
        std::string dummyfiledata;
        send(msg_type_upload_resp,seq_++,file_msg_error_complete,filename,offset,filesize,dummyfiledata);
        LOG_INFO<<"Response to client: cmd=msg_type_upload_resp,errorcode=file_msg_error_complete, filename"<<filename
        <<" ,offset: "<<offset<<" ,filesize: "<<filesize<<" ,client: "<<conn->peerAddress().toIpPort();
        return true;
    }
    if(offset==0)
    {
        std::string filenameLocal=strFileBaseDir_;
        filenameLocal+=filename;
        fp_=fopen(filenameLocal.c_str(),"wb");
        if(fp_==NULL)
        {
            LOG_ERROR<<"fopen file error, filename: "<<filename<<" ,client: "<<conn->peerAddress().toIpPort();
            return false;
        }
        bFileUploading_= true;
    }
    else
    {
        if(fp_==NULL)
        {
            resetFile();
            LOG_ERROR<<"file pointer should be not null, filename: "<<filename<<", offset: "<<offset<<" ,client: "
            <<conn->peerAddress().toIpPort();
            return false;
        }
    }
    if(fseek(fp_,offset,SEEK_SET)==-1)
    {
        int savaError=errno;
        LOG_ERROR<<"fseek file error, filename: "<<filename<<" ,errno: "<<savaError<<" ,errorinfo: "<<strerror_tl(savaError)
        <<" fp_: "<<fp_<<" ,client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    if(fwrite((char*)filedata.c_str(),1,filedata.size(),fp_)!=filedata.size())
    {
        int savaError=errno;
        LOG_ERROR<<"fwrite file error, filename: "<<filename<<" ,errno: "<<savaError<<" ,errorinfo: "<<strerror_tl(savaError)
                 <<" fp_: "<<fp_<<" ,client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    if(fflush(fp_)!=0)
    {

        int savaError=errno;
        LOG_ERROR<<"fflush file error, filename: "<<filename<<" ,errno: "<<savaError<<" ,errorinfo: "<<strerror_tl(savaError)
                 <<" fp_: "<<fp_<<" ,client: "<<conn->peerAddress().toIpPort();
        return false;
    }

    int32_t errorcode=file_msg_error_progress;
    if(offset+(int64_t)filedata.length()==filesize)
    {
        offset=filesize;
        errorcode=file_msg_error_complete;
        Singleton<FileManager>::instance().addFile(filename.c_str());
        resetFile();
    }
    string dummyfiledatax;
    send(msg_type_upload_resp, seq_, errorcode, filename, offset, filesize, dummyfiledatax);

    std::string errorcodestr = "file_msg_error_progress";
    if (errorcode == file_msg_error_complete)
        errorcodestr = "file_msg_error_complete";

    LOG_INFO<<"Response to client: cmd=msg_type_upload_resp, errorcode: "<<errorcodestr<<", filename: "<<filename
    <<", offset: "<<offset<<", filesize: "<<filesize<<", upload percent: "<<(int32_t)(offset * 100 / filesize)
    <<", client: "<<conn->peerAddress().toIpPort();
    return true;
}
