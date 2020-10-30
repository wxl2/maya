//
// Created by wxl on 2020/10/13.
//

#ifndef MAYA_LOGFILE_H
#define MAYA_LOGFILE_H

#include "Types.h"
#include "nocopyable.h"
#include "Logging.h"
#include <mutex>
#include <unistd.h>

#include <memory>

namespace maya{

namespace FileUtil{
    class AppendFile;
}//namespace FileUtil

    class LogFile:nocopyable
    {
    public:
        LogFile(const string& basename,
                off_t rollSize,
                bool threadSafe = true,
                int flushInterval = 3,
                int checkEveryN = 1024);
        ~LogFile();

        void append(const char* logline, int len);
        void flush();
        bool rollFile();

    private:
        void append_unlocked(const char* logline, int len);

        static string getLogFileName(const string& basename, time_t* now);

        const string basename_;
        const off_t rollSize_;
        const int flushInterval_;
        const int checkEveryN_;

        int count_;

        std::unique_ptr<std::mutex> mutex_;
        time_t startOfPeriod_;//文件创建时间，都为每天的0时0分
        time_t lastRoll_;//上次文件滚动时间
        time_t lastFlush_;//上次刷新时间
        std::unique_ptr<FileUtil::AppendFile> file_;

        const static int kRollPerSeconds_ = 60*60*24;
    };
}//namespace maya


#endif //MAYA_LOGFILE_H
