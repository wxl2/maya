//
// Created by wxl on 2020/10/13.
//

#ifndef MAYA_LOGGING_H
#define MAYA_LOGGING_H

#include "Types.h"
#include "Timestamp.h"
#include "LogStream.h"
#include <thread>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace maya{
    class Logger {
    public:
        enum LogLevel
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };
    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char (&arr)[N])
        :data_(arr),
        size_(N)
        {
            const char* slash = strrchr(data_, '/');
            if (slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename)
        :data_(filename)
        {
            const char* slash = strrchr(filename, '/');
            if (slash)
            {
                data_ = slash + 1;
            }
            size_=static_cast<int>(strlen(data_));
        }

    public:
        const char *data_;
        int size_;
    };//end of SoureFile

        Logger(SourceFile file, int line);
        Logger(SourceFile file, int line, LogLevel level);
        Logger(SourceFile file, int line, LogLevel level, const char* func);
        Logger(SourceFile file, int line, bool toAbort);
        ~Logger();

        LogStream& stream() { return impl_.stream_; }

        static LogLevel logLevel();
        static void setLogLevel(LogLevel level);

        typedef void (*OutputFunc)(const char* msg, int len);
        typedef void (*FlushFunc)();
        static void setOutput(OutputFunc);
        static void setFlush(FlushFunc);

    private:

        class Impl
        {
        public:
            typedef Logger::LogLevel LogLevel;
            Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
            void formatTime();
            void finish();

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;
        };

        Impl impl_;
    };//end of Logger
    const char* strerror_tl(int savedErrno);

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel()
    {
        return g_logLevel;
    }


#define LOG_TRACE if (maya::Logger::logLevel() <= maya::Logger::TRACE) \
  maya::Logger(__FILE__, __LINE__, maya::Logger::TRACE, __PRETTY_FUNCTION__).stream()
#define LOG_DEBUG if (maya::Logger::logLevel() <= maya::Logger::DEBUG) \
  maya::Logger(__FILE__, __LINE__, maya::Logger::DEBUG, __PRETTY_FUNCTION__).stream()
#define LOG_INFO if (maya::Logger::logLevel() <= maya::Logger::INFO) \
  maya::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN maya::Logger(__FILE__, __LINE__, maya::Logger::WARN).stream()
#define LOG_ERROR maya::Logger(__FILE__, __LINE__, maya::Logger::ERROR).stream()
#define LOG_FATAL maya::Logger(__FILE__, __LINE__, maya::Logger::FATAL).stream()
#define LOG_SYSERR maya::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL maya::Logger(__FILE__, __LINE__, true).stream()

#define CHECK_NOTNULL(val) \
  ::maya::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
    template <typename T>
    T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
    {
        if (ptr == NULL)
        {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }
}//namespace maya



#endif //MAYA_LOGGING_H
