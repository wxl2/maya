//
// Created by wxl on 2020/10/14.
//

#include "../base/LogFile.h"
#include "../base/AsyncLogging.h"

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

off_t kRollSize = 500*1000*1000;

maya::AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void bench(bool longLog)
{
    maya::Logger::setOutput(asyncOutput);

    int cnt = 0;
    const int kBatch = 1000;
    maya::string empty = " ";
    maya::string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t)
    {
        maya::Timestamp start = maya::Timestamp::now();
        for (int i = 0; i < kBatch; ++i)
        {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                     << (longLog ? longStr : empty)
                     << cnt;
            ++cnt;
        }
        maya::Timestamp end = maya::Timestamp::now();
        printf("%f\n", timeDifference(end, start)*1000000/kBatch);
        struct timespec ts = { 0, 500*1000*1000 };
        nanosleep(&ts, NULL);
    }
}

void func()
{
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz ";
}
int main(int argc, char* argv[])
{
//    {
//        // set max virtual memory to 2GB.
//        size_t kOneGB = 1000*1024*1024;
//        rlimit rl = { 2*kOneGB, 2*kOneGB };
//        setrlimit(RLIMIT_AS, &rl);
//    }

    printf("pid = %d\n", getpid());

    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof name - 1);
    maya::AsyncLogging log(::basename(name), kRollSize);
    maya::Logger::setOutput(asyncOutput);
    log.start();
    g_asyncLog = &log;
    std::thread func1(func);
    std::thread func2(func);


    bool longLog = argc > 1;
    func1.join();
    func2.join();
    //bench(longLog);
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz ";
    log.stop();
    //sleep(5);
    return 0;
}
