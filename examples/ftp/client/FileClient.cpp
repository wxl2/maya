//
// Created by wxl on 2020/12/2.
//

#include "FileClient.h"
#include "FileMsg.h"
#include "utils/StringUtil.h"
#include "utils/ProtocolStream.h"
#include <iostream>
using namespace maya;
using namespace maya::net;

void FileClient::connect()
{
    client_.connect();
}

void FileClient::disconnect()
{
    client_.disconnect();
}

FileClient::FileClient(EventLoop *loop, const InetAddress &address)
:client_(loop,address,"ftpClient"),seq_(0),fp_(NULL),
currentDownloadFileOffset_(0),
currentDownloadFileSize_(0)
{
    client_.setConnectionCallback(std::bind(&FileClient::onConnection,this,_1));
    client_.setMessageCallback(std::bind(&FileClient::onRead,this,_1,_2,_3));
}

void FileClient::onConnection(const TcpConnectionPtr &conn)
{
    LOG_INFO<<conn->localAddress().toIpPort()<<" -> "<<conn->peerAddress().toIpPort();
    std::lock_guard<std::mutex> lock(mutex_);
    if(conn->connected())
    {
        conn_=conn;
    }
    else
    {
        conn_.reset();
    }
}

void FileClient::onRead(const std::shared_ptr<TcpConnection>& conn,Buffer* buffer,Timestamp receivTime)
{
    //不管服务器出错的情况,直接接收数据包
    file_msg_header header;
    memcpy(&header,buffer->peek(),sizeof (file_msg_header));

    buffer->retrieve(sizeof (file_msg_header));
    std::string buf;
    buf.append(buffer->peek(),header.packagesize);
    buffer->retrieve(header.packagesize);
    if(!process(conn,buf.c_str(),buf.size()))
    {
        LOG_ERROR<<"Perocess error, TcpConnection: "<<conn->peerAddress().toIpPort();
        //不关闭连接,直接返回
        return;
    }
}

bool FileClient::process(const std::shared_ptr<TcpConnection> &conn, const char *inbuf, size_t length)
{
    detail::BinaryStreamReader readerSteam(inbuf,length);
    int32_t cmd;
    ///命令头,文件包序列号,错误码,文件名,当前传输数据包在文件中的偏移量,文件大小,文件内容
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
    int32_t errorcode;
    if(!readerSteam.ReadInt32(errorcode))
    {
        LOG_ERROR<<"Read errorcode error, server: "<<conn->peerAddress().toIpPort();
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

    LOG_INFO<<"Request from server: "<<conn->peerAddress().toIpPort()<<" ,cmd: "<<cmd
            <<" ,seq: "<<seq_<<" ,filename: "<<filename<<" ,offset: "<<offset<<" ,filedatalength: "<<filedataLength;

    switch (cmd)
    {
        case msg_type_list_resp:
            return onListFile(filedata,filedataLength);
        case msg_type_download_resp:
            return onDownloadFileRequest(filename,offset,filesize,filedata);
        case msg_type_upload_resp:
            return onUploadFileRequest(filename);
        default:
            LOG_ERROR<<"unknown cmd type, client: "<<conn->peerAddress().toIpPort();
            return false;
    }
}


bool FileClient::processLine(std::string& cmdline)
{
    /*!
     * 命令格式->客户端需要将所有命令转换为小写再发送给服务器
     * list: 获取文件列表
     * get filename: 获取文件名为filename的文件
     * send filename: 上传文件名为filename的文件
     * cd dir: 切换到dir这个目录 TODO:待实现
     */
    detail::StringUtil::trimLeft(cmdline);
    detail::StringUtil::trimRigtht(cmdline);
    char c=cmdline[0];
    bool ret= false;
    switch (c)
    {
        case 'l':
        {
            std::unique_lock<std::mutex> lock(mutex_);
            ret = onListFile(cmdline,cmdline.length());
            if(ret)
                cond_.wait(lock);
            break;
        }
        case 'g':
        {
            std::unique_lock<std::mutex> lock(mutex_);
            ret = onDownload(cmdline);
            if(ret)
                cond_.wait(lock);
            break;
        }
        case 's':
        {
            std::unique_lock<std::mutex> lock(mutex_);
            ret = onUpload(cmdline);
            if(ret)
                cond_.wait(lock);
            break;
        }
        default:
            std::cout<<"unknown cmd type "<<cmdline<<std::endl;
            return false;
    }
    return ret;
}

bool FileClient::onListFile(std::string& filedata,size_t length)
{
    if(length<=5)
    {
        send(msg_type_list_req,seq_,"list",0,0,"list");
        return true;
    }
    else
    {
        std::vector<std::string> vec;
        detail::StringUtil::split(filedata,vec,",");
        int i=0;
        for(const auto& iter:vec)
        {
            i++;
            std::cout<<iter<<"  ";
            if(i%5==0)
                std::cout<<std::endl;
        }
        std::cout<<std::endl;
        cond_.notify_all();
        return true;
    }
}

void FileClient::send(int32_t cmd, int32_t seq,const string &filename, int64_t offset, int64_t filesize,
                 const string &filedata)
{
    std::string outbuf;
    maya::detail::BinaryStreamWriter writerStream(&outbuf);
    writerStream.WriteInt32(cmd);
    writerStream.WriteInt32(seq);
    writerStream.WriteString(filename);
    writerStream.WriteInt64(offset);
    writerStream.WriteInt64(filesize);
    writerStream.WriteString(filedata);

    writerStream.Flush();
    sendPackage(outbuf.c_str(),outbuf.length());
}

void FileClient::sendPackage(const char *body, int64_t bodylength)
{
    string strPackageData;
    file_msg_header header = { (int64_t)bodylength };
    strPackageData.append((const char*)&header, sizeof(header));
    strPackageData.append(body, bodylength);
    if(!conn_)
    {
        LOG_ERROR<<"TcpConnection is NULL";
        return;
    }
    LOG_INFO<<"Send data, package length: "<<strPackageData.length()<<" body length: "<<bodylength;
    conn_->send(strPackageData.c_str(), strPackageData.length());
}

bool FileClient::onDownload(const string &filename)
{
    std::string name=filename.substr(3);
    detail::StringUtil::trim(name);
    if(name.empty())
    {
        std::cout<<"input the right filename"<<std::endl;
        return false;
    }
    std::string buf;
    detail::BinaryStreamWriter writerStream(&buf);
    ///命令头,文件包序列号,文件名,当前传输数据包在文件中的偏移量,文件大小,文件内容,这里仅前3个字段有效
    send(msg_type_download_req,seq_++,name,0,0,"get");
    return true;
}

bool FileClient::onDownloadFileRequest(const string &filename, int64_t offset, int64_t filesize, const string &filedata)
{
    if(offset==0)
    {
        fp_=fopen(filename.c_str(),"wb");
        if(fp_==NULL)
        {
            std::cout<<"fopen file error, filename: "<<filename<<" try again"<<std::endl;
            return false;
        }
    }
    else
    {
        if(fp_==NULL)
        {
            std::cout<<"file pointer should not be null, filename: "<<filename<<" try again"<<std::endl;
            resetFile();
            return false;
        }
    }
    if(fseek(fp_,offset,SEEK_SET)==-1)
    {
        std::cout<<"seek file error, filename: "<<filename<<" try again"<<std::endl;
        resetFile();
        return false;
    }
    if (fwrite((char*)filedata.c_str(), 1, filedata.length(), fp_) != filedata.length())
    {
        std::cout<<"fwrite file error, filename: "<<filename<<" try again"<<std::endl;
        resetFile();
        return false;
    }
    if(fflush(fp_)!=0)
    {
        std::cout<<"fflush file error try again"<<std::endl;
        resetFile();
        return false;
    }
    char buf[64]={0};
    int num=(offset+filedata.length())*100/filesize;
    int i=0;
    for(i=0;i<num/2;i++)
        buf[i]='=';
    buf[i+1]='>';
    printf("\rdownload:[%-50s]%d%% ", buf, num);
    if(offset+(int64_t)filedata.length()==filesize)
    {
        std::cout<<"\n"<<filename<<" download ok"<<std::endl;
        cond_.notify_all();
        return true;
    }
    send(msg_type_download_req,seq_++,filename,offset,filesize,"get");
    return true;
}

void FileClient::resetFile()
{
    if (fp_ != NULL)
    {
        fclose(fp_);
        currentDownloadFileSize_=0;
        currentDownloadFileOffset_=0;
        fp_=NULL;
    }
}

bool FileClient::onUploadFileRequest(const string &filename)
{
    if(currentDownloadFileSize_==currentDownloadFileOffset_
    &&currentDownloadFileOffset_!=0&&currentDownloadFileSize_!=0)
    {
        std::cout<<"upload "<<filename<<" ok "<<std::endl;
        resetFile();
        cond_.notify_all();
        return true;
    }
    if (fp_ == NULL)
    {
        fp_ = fopen(filename.c_str(), "rb");
        if (fp_ == NULL)
        {
            perror("open error");
            std::cout << "open file error, try again" << std::endl;
            return false;
        }
        if (fseek(fp_, 0, SEEK_END) == -1)
        {
            std::cout << "fseek file error,try again" << std::endl;
            return false;
        }
        //获得文件大小
        currentDownloadFileSize_ = ftell(fp_);
        if (currentDownloadFileSize_ <= 0) {
            std::cout << "get filesize error, try again" << std::endl;
            return false;
        }
        //将文件移动只开头
        if (fseek(fp_, 0, SEEK_SET) == -1)
        {
            std::cout << "filesize seek start error, try again" << std::endl;
            return false;
        }
    }

    std::string filedata;
    int64_t currentSendSize=512*1024;
    char buf[512*1024]={0};
    if(currentDownloadFileSize_<=currentDownloadFileOffset_+currentSendSize)
    {
        currentSendSize=currentDownloadFileSize_-currentDownloadFileOffset_;
    }
    if(currentSendSize<=0||fread(buf,currentSendSize,1,fp_)!=1)
    {
        int saveErrno=errno;
        std::cout<<"fread error, filename: "<<filename<<" errno: "<<saveErrno<<" errorinfo: "<<strerror_tl(saveErrno)<<std::endl;
        return false;
    }

    int sendOffset=currentDownloadFileOffset_;
    currentDownloadFileOffset_+=currentSendSize;
    filedata.append(buf,currentSendSize);

    char buffer[64]={0};
    int num=currentDownloadFileOffset_*100/currentDownloadFileSize_;
    int i=0;
    for(i=0;i<num/2;i++)
        buffer[i]='=';
    buffer[i+1]='>';
    printf("\rupload:[%-50s]%d%% ", buffer, num);
    send(msg_type_upload_req,seq_++,filename,sendOffset,currentDownloadFileSize_,filedata);

    return true;
}

bool FileClient::onUpload(const string &filename)
{
    std::string name=filename.substr(4);
    detail::StringUtil::trim(name);
    if(name.empty())
    {
        std::cout<<"input the right filename"<<std::endl;
        return false;
    }
    return onUploadFileRequest(name);
}

