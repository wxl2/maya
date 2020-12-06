//
// Created by wxl on 2020/12/1.
//
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <iostream>
#include "base/LogFile.h"
#include "net/InetAddress.h"
#include "net/EventLoopThread.h"
#include "FileClient.h"

using namespace maya;
using namespace maya::net;
std::unique_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

void flushFunc()
{
    g_logFile->flush();
}

FileClient *g_client=NULL;
EventLoopThread* g_loop;
void prog_exit(int sig)
{
    std::cout<<"bye"<<std::endl;
    if(g_client!=NULL)
    {
     g_client->disconnect();
    }
    if(g_loop!=NULL)
    {
        g_loop->stopLoop();
    }
    exit(0);
}
int main(int argc,char* argv[])
{
    g_logFile.reset(new LogFile("ftpClient", 200*1000));
    Logger::setOutput(outputFunc);
    Logger::setFlush(flushFunc);
    signal(SIGTERM,prog_exit);
    signal(SIGINT,prog_exit);
//    if(argc<3)
//    {
//        printf("Usage: %s ip port",argv[0]);
//        exit(1);
//    }
    uint16_t port=static_cast<uint16_t>(8001/*atoi(argv[2])*/);
    InetAddress address(/*argv[1]*/"127.0.0.1",port);
    EventLoopThread loopThread;
    g_loop=&loopThread;
    FileClient client(loopThread.startLoop(),address);
    g_client=&client;
    g_client->connect();
    char buf[1024];
    while (true)
    {
        std::cout << ">";
        std::cin.getline(buf, 1023);
        if(strncmp(buf,"quit",5)==0)
        {
            prog_exit(0);
            break;//no use
        }
        std::string str(buf);
        if(str.size()!=0)
            client.processLine(str);
    }
    return 0;
}