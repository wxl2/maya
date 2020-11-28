//
// Created by wxl on 2020/10/22.
//
#include "net/EventLoop.h"
#include "net/Channel.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <thread>
#include <sys/timerfd.h>

//void threadFunc()
//{
//    printf("threadFunc(): pid = %d , tid = %d\n",getpid(),syscall(SYS_gettid));
//    maya::net::EventLoop loop;
//    loop.loop();
//}
//
//maya::net::EventLoop* g_loop;
//
//void threadFunc2()
//{
//    g_loop->loop();
//}
//int main()
//{
//    printf("main(): pid = %d , tid = %d\n",getpid(),syscall(SYS_gettid));
//    maya::net::EventLoop loop;
//    g_loop=&loop;
//    std::thread th(threadFunc2);
//
//    th.join();
//    pthread_exit(NULL);
//
//}

maya::net::EventLoop* g_loop;

void timeout()
{
    printf("Timeout\n");
    g_loop->quit();
}

int main()
{
    maya::net::EventLoop loop;
    g_loop=&loop;
    int timefd=::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    maya::net::Channel channel(&loop,timefd);
    channel.setReadCallback(std::bind(timeout));
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong,sizeof(howlong));
    howlong.it_value.tv_sec=5;
    ::timerfd_settime(timefd,0,&howlong,NULL);

    loop.loop();
    ::close(timefd);
}
