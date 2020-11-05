//
// Created by wxl on 2020/10/22.
//

#include "EventLoop.h"
#include "SocketsOps.h"
#include "Poller.h"
#include "Channel.h"
#include <sstream>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <unistd.h>

using namespace maya;
using namespace maya::net;

__thread EventLoop* t_loopInThisThread=0;

const int kPollTimeMs=10000;

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}
EventLoop::EventLoop()
:looping_(false),
threadId_(std::this_thread::get_id(),::syscall(SYS_gettid))
{
    std::stringstream ss;
    ss<<"EventLoop created "<<this<<" in thread "<<threadId_.second;
    LOG_TRACE<<ss.str().c_str();
    poller_.reset(new Poller(this));
    if(t_loopInThisThread)
    {
        ss.clear();
        ss<<"Another EventLoop "<<t_loopInThisThread<<" exists in this thread "<<threadId_.second;
        LOG_FATAL<<ss.str().c_str();
    }
    else
    {
        t_loopInThisThread=this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread=NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_= true;
    quit_= false;
    while(!quit_)
    {
        activeChanels_.clear();
        poller_->poll(kPollTimeMs,&activeChanels_);
        for(ChannelList::iterator it=activeChanels_.begin();it!=activeChanels_.end();++it)
        {
           //FIXME (*it)->handleEvent();
        }
        doPendingFunctors();
    }
    LOG_TRACE<<"EventLoop "<< this<<" stop running";
    looping_= false;
}

void EventLoop::abortNotInLoopThread()
{
    std::stringstream ss;
    ss<< "EventLoop::abortNotInLoopThread - EventLoop " << this
      << " was created in threadId_ = " << threadId_.second
      << ", current thread id = " <<::syscall(SYS_gettid);
    LOG_FATAL<<ss.str().c_str();
}

void EventLoop::quit()
{
    quit_= true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerloop()== this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb)
{
    return timerQueue_->addTimer(cb,time,0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(),delay));
    return runAt(time,cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(),interval));
    return timerQueue_->addTimer(cb,time,interval);
}

void EventLoop::runInLoop(EventLoop::Functor cb)
{
    if(isInLoopThread()){
        cb();
    } else{
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if(!isInLoopThread()||callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(int i=0;i<functors.size();++i)
    {
        functors[i]();
    }
    callingPendingFunctors_= false;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerloop());
    assertInLoopThread();
    if(eventHandling_)
    {
        assert(currentActiveChannel_==channel||
        std::find(activeChanels_.begin(),activeChanels_.end(),channel)==activeChanels_.end());
    }
    poller_->removeChannel(channel);
}
