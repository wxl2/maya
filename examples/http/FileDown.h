//
// Created by wxl on 2020/11/16.
//

#ifndef MAYA_FILEDOWN_H
#define MAYA_FILEDOWN_H

#include "base/nocopyable.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "base/Types.h"
#include <mutex>
#include <vector>

namespace maya{
namespace net{
    class FileDown:nocopyable
    {
    public:
        explicit FileDown(const string url);
        ~FileDown();
        void onRequest(const HttpRequest& req, HttpResponse* resp);
        int readDir(std::vector<std::string>& list,const string url);
        int judgePath(const string path);
        int readFile(string filename,int size,string& buf);
        void isDir(const string path,HttpResponse* resp);
        void isFile(const string filename,int size,HttpResponse* resp);
        void fileNoExist(const string filename,HttpResponse* resp);
        void serverError(int statusCode,HttpResponse *resp);
        void setBenchmark(bool on)
        { benchmark_=on;}
    private:
        std::mutex mutex_;
        bool benchmark_;
        std::string url_;
//        std::vector<std::string> filelist_;
    };
}//namespace net
}//namespace maya
#endif //MAYA_FILEDOWN_H
