//
// Created by wxl on 2020/10/13.
//

#include "LogFile.h"
#include "FileUtil.h"

#include <time.h>
#include <stdio.h>
#include <assert.h>

namespace maya{

    LogFile::LogFile(const string &basename,
                     off_t rollSize,
                     bool threadSafe,
                     int flushInterval,
                     int checkEveryN)
    :basename_(basename),
     rollSize_(rollSize),
     flushInterval_(flushInterval),
     checkEveryN_(checkEveryN),
     count_(0),
     mutex_(threadSafe ? new std::mutex : NULL),
     startOfPeriod_(0),
     lastRoll_(0),
     lastFlush_(0)
    {
        assert(basename.find('/')==string::npos);
        rollFile();
    }

    void LogFile::append(const char *logline, int len)
    {
        if(mutex_)
        {
            std::lock_guard<std::mutex> lock(*mutex_);
            append_unlocked(logline,len);
        }
        else
        {
            append_unlocked(logline,len);
        }
    }

    void LogFile::flush()
    {
        if(mutex_)
        {
            std::lock_guard<std::mutex> lock(*mutex_);
            file_->flush();
        }
        else
        {
            file_->flush();
        }
    }

    bool LogFile::rollFile()
    {
        time_t now = 0;
        string filename = getLogFileName(basename_, &now);
        time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;//除了之后再乘，正好消掉大于一天小于两天的部分

        if (now > lastRoll_)
        {
            lastRoll_ = now;
            lastFlush_ = now;
            startOfPeriod_ = start;
            file_.reset(new FileUtil::AppendFile(filename));
            return true;
        }
        return false;
    }

    void LogFile::append_unlocked(const char *logline, int len)
    {
        file_->append(logline,len);
        if(file_->writtenBytes()>rollSize_)
        {
            rollFile();
        }
        else
        {
            ++count_;
            if(count_>checkEveryN_)
            {
                count_=0;
                time_t now=::time(NULL);
                time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
                //整数除法取整，若小于一天乘后再除与startOfPeriod_相等，多于一天则多kRollPerSeconds_，大于startOfPeriod_
                if(thisPeriod_!=startOfPeriod_)
                {
                    rollFile();
                }
                else if(now - lastFlush_ > flushInterval_)//刷新缓冲
                {
                    lastFlush_=now;
                    file_->flush();
                }
            }
        }
    }

    string LogFile::getLogFileName(const string &basename, time_t *now)
    {
        string filename;
        filename.reserve(basename.size() + 64);
        filename = basename;

        char timebuf[32];
        struct tm tm;
        *now = time(NULL);
        gmtime_r(now, &tm); // FIXME: localtime_r ?
        strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
        filename += timebuf;

        char buf[256];
        if (::gethostname(buf, sizeof buf) == 0)
        {
            buf[sizeof(buf)-1] = '\0';
        }
        else
        {
            strncpy(buf,"unknownhost",12) ;
        }
        string hostname(buf);
        filename += hostname;

        char pidbuf[32];
        snprintf(pidbuf, sizeof pidbuf, ".%d",::getpid());
        filename += pidbuf;

        filename += ".log";

        return filename;
    }

    LogFile::~LogFile() = default;


}//namespace maya
