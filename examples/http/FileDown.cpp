//
// Created by wxl on 2020/11/16.
//

#include "FileDown.h"
#include "base/Logging.h"
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace maya;
using namespace maya::net;
extern char favicon[555];

FileDown::FileDown(const string url)
:benchmark_(false),
url_(url)
{
}

void FileDown::doGet(const HttpRequest &req, HttpResponse *resp)
{
    std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
    if (!benchmark_)
    {
        const std::map<string, string>& headers = req.headers();
        for (const auto& header : headers)
        {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }
    if (req.path() == "/")
    {
        std::vector<std::string> filelist_;
        resp->setStatusCode(HttpResponse::k200Ok);
        if (filelist_.empty()) {
            if (readDir(filelist_, url_) < 0) {
                serverError(HttpResponse::k500ServerError, resp);
                return;
            }
        }
        resp->setStatusMessage("OK");
        resp->setContentType("text/html;charset=utf-8");
        resp->setContentLanguage("zh-CN");
        resp->addHeader("Server", "Muduo");
        string outhtml;
        for (int i = 0; i < filelist_.size(); ++i) {
            //<a href="http://www.w3school.com.cn">W3School</a>
            outhtml += "<a href=\"";
            outhtml += filelist_[i];
            outhtml += "\">";
            outhtml += filelist_[i] + "</a><br/>";
        }
        resp->setBody(outhtml);
    } else if (req.path() == "/favicon.ico") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("image/png");
        resp->setBody(string(favicon, sizeof favicon));
    } else {
        int ret = judgePath(req.path());
        if (ret > 0)
            isFile(req.path(), ret, resp);
        else if (ret == 0)
            isDir(req.path(), resp);
        else if (ret < 0)
            fileNoExist(req.path(), resp);
    }
}

int FileDown::readDir(std::vector<std::string>& list,const string url)
{
    DIR             *dp;
    struct dirent   *dirp;
    if((dp=opendir(url.c_str()))==NULL)
    {
        LOG_ERROR<<"FileDown::readDir";
        return -1;
    }
    while((dirp=readdir(dp))!=NULL)
    {
        if((strncmp(dirp->d_name,".",2)==0)
            ||(strncmp(dirp->d_name,"..",3))==0)
            continue;
        list.emplace_back(dirp->d_name);
    }

    closedir(dp);
    return 0;
}



void FileDown::serverError(int statusCode,HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k500ServerError);
    resp->setStatusMessage("Server Error");
    resp->setCloseConnection(true);
    return;
}

int FileDown::judgePath(const string path)
{
    //ret==0    path所指向的是一个目录
    //ret<0     path指向的目录或文件不存在
    //ret>0     path指向的是一个文件,且ret==filesize
    struct stat buf;
    string name;
    name.append(".");
    name.append(path);
    if(lstat(name.c_str(),&buf)<0)
    {
        LOG_SYSERR<<"FileDown::judgePath():";
        return -1;
    }

    if(S_ISDIR(buf.st_mode))
        return 0;
    return buf.st_size;
}

void FileDown::isDir(const string path, HttpResponse *resp)
{
    std::vector<std::string> filelist;
    string filepath;
    filepath.append(".");
    filepath.append(path);
    if(readDir(filelist,filepath)<0)
    {
        serverError(HttpResponse::k500ServerError,resp);
        return;
    }
    resp->setStatusMessage("OK");
    resp->setContentType("text/html;charset=utf-8");
    resp->setContentLanguage("zh-CN");
    resp->addHeader("Server", "Muduo");
    string outhtml;
    filepath.erase(filepath.begin(),filepath.begin()+2);
    filepath.append("/");
    for(int i=0;i<filelist.size();++i)
    {
        //<a href="http://www.w3school.com.cn">W3School</a>
        outhtml+="<a href=\"";
        outhtml+=filepath;
        outhtml+=filelist[i];
        outhtml+="\">";
        outhtml+=filelist[i]+"</a><br/>";
    }
    resp->setBody(outhtml);
}

void FileDown::isFile(const string filename,int size,HttpResponse *resp)
{
    string contentType;
    size_t pos=filename.find_last_of('.');
    if(pos!=string::npos)
    {
        string type=filename.substr(pos);
        if(type==".jpg")
            contentType="image/jpeg";
        else if(type==".png")
            contentType="image/png";
        else if(type==".gif")
            contentType="image/gif";
        else if(type==".html")
            contentType="text/html;charset=utf-8";
        else
            contentType="application/octet-stream";
    }
    else
        contentType="application/octet-stream";

    string filebuf;
    if(readFile(filename,size,filebuf)<0)
    {
        serverError(HttpResponse::k500ServerError,resp);
        return;
    }
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType(contentType);
    resp->setBody(filebuf);
}

int FileDown::readFile(const string filename,int size,string& buf)
{
    char buffer[1024*64];
    int len=0;
    string path;
    path.append(".");
    path.append(filename);
    //当多个线程读同一个文件时需要加锁
    std::lock_guard<std::mutex> lock(mutex_);
    int fd=::open(path.c_str(),O_RDONLY);
    if(fd<0)
    {
        LOG_SYSERR<<"FileDown::readFile()::open()";
        return -1;
    }
    while (len<size)
    {
        int n=::read(fd,buffer,sizeof(buffer));
        if(n<0)
        {
            LOG_SYSERR<<"FileDown::readFile()::read()";
            return -1;
        }
        else
        {
            buf.append(buffer,n);
            len+=n;
        }
    }
    return 0;
}


void FileDown::fileNoExist(const string filename, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("NotFound");
    resp->setContentType("text/html;charset=utf-8");
    resp->setContentLanguage("zh-CN");
    resp->addHeader("Server", "Muduo");
    string outhtml("<html><head><title>404</title></head>"
                   "<body><p>file: "+filename+
                   " not exist</p>"
                   "</body></html>");
    resp->setBody(outhtml);
}

void FileDown::doPost(const HttpRequest &req, HttpResponse *resp)
{
    //TODO:finish ths Post Method handle
    resp->setStatusMessage("OK");
    resp->setContentType("text/html;charset=utf-8");
    resp->setContentLanguage("zh-CN");
    resp->addHeader("Server", "Muduo");
    string out;
    out.append(req.path()+" ");
    out.append(req.getBody());
    resp->setBody(out);
}

FileDown::~FileDown()=default;
