//
// Created by wxl on 2020/11/30.
//
#include "FileServer.h"
#include "FileManager.h"
#include "base/Singleton.h"
#include "base/AsyncLogging.h"
#include "utils/DaemonRun.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

using namespace maya;
using namespace maya::net;

off_t kRollSize = 500*1000*1000;
AsyncLogging* g_asyncLog=NULL;

void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

EventLoop g_loop;
void prog_exit(int sig)
{
    std::cout<<"program recv signal ["<<sig<<"] to exit."<<std::endl;
    if(g_asyncLog!=NULL)
        g_asyncLog->stop();
    g_loop.quit();
    exit(0);
}
int main(int argc,char* argv[])
{
    Logger::setOutput(asyncOutput);
    signal(SIGCHLD,SIG_DFL);
    signal(SIGPIPE,SIG_IGN);
    signal(SIGTERM,prog_exit);
    signal(SIGINT,prog_exit);

    int ch;
    bool bdaemon = false;
    while ((ch = getopt(argc, argv, "d")) != -1)
    {
        switch (ch)
        {
            case 'd':
                bdaemon = true;
                break;
        }
    }

    if (bdaemon)
        maya::detail::daemon_run();
    AsyncLogging log("ftpServer", kRollSize);
    log.start();
    g_asyncLog = &log;
    Singleton<FileManager>::instance().init("./");
    Singleton<FileServer>::instance().init("127.0.0.1",8001,&g_loop,"./");
    g_loop.loop();
    LOG_INFO<<"ftpserver exit";
    return 0;
}