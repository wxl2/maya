//
// Created by wxl on 2020/10/12.
//

#ifndef MAYA_FILEUTIL_H
#define MAYA_FILEUTIL_H

#include "nocopyable.h"
#include <sys/types.h>
#include "Types.h"
#include <stdio.h>
#include <thread>

namespace maya{
    static __thread char t_errnobuf[512];
}

namespace maya{
namespace FileUtil{
    //非线程安全，在一个线程内单独调用
    class AppendFile:nocopyable{
    public:
        explicit AppendFile(string filename);
        ~AppendFile();

        void append(const char* logline,size_t len);
        void flush();

        off_t writtenBytes()const{return writtenBytes_;}
    private:
        size_t write(const char* logline,size_t len);

        FILE *fp_;
        char buffer_[60*1024];
        off_t writtenBytes_;
    };
}//namespace FileUtil
}//namespace maya



#endif //MAYA_FILEUTIL_H
