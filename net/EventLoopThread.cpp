//
// Created by wxl on 2020/11/5.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace maya;
using namespace maya::net;

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb, const string &name)
:loop_(NULL),
exiting_(false),
callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_= true;
    if(loop_!=NULL)
    {
        loop_->quit();
        thread_->join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.reset(new std::thread(std::bind(&EventLoopThread::threadFunc,this)));
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_==NULL)
        {
            cond_.wait(lock);
        }
        return loop_;
    }
}

void EventLoopThread::stopLoop()
{
    if(loop_!=NULL)
        loop_->quit();
    thread_->join();
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;

    if(callback_)
        callback_(&loop);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_all();
    }
    loop.loop();
    loop_=NULL;
}
