//
// Created by wxl on 2020/10/12.
//

#include "FileUtil.h"

using namespace maya::FileUtil;

AppendFile::AppendFile(std::string filename):
fp_(::fopen(filename.c_str(),/*"ae"*/"r")),// 'e' for O_CLOEXEC
writtenBytes_(0)
{
    assert(fp_);
    ::setbuffer(fp_,buffer_,sizeof(buffer_));
}

AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

void AppendFile::append(const char *logline, size_t len)
{
    size_t n=write(logline,len);
    size_t remain=len-n;//剩余长度

    while (remain>0)
    {
        size_t x=write(logline+n,len-n);
        if(x==0)
        {
            int err= ferror(fp_);
            if(err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_r(err,t_errnobuf,sizeof(t_errnobuf)));
            }
            break;
        }
        n+=x;
        remain=len-n;
    }
    writtenBytes_+=len;
}

void AppendFile::flush()
{
    ::fflush(fp_);
}

size_t AppendFile::write(const char *logline, size_t len)
{
    return fwrite_unlocked(logline,1,len,fp_);
}


