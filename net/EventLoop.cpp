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
#include <signal.h>
#include <sys/eventfd.h>
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
threadId_(std::this_thread::get_id(),::syscall(SYS_gettid)),
quit_(false),
eventHandling_(false),
iteration_(0),
poller_(Poller::newDefaultPoller(this)),
timerQueue_(new TimerQueue(this)),
wakeupFd_(EventLoop::createWakeupfd()),
wakeupChannel_(new Channel(this,wakeupFd_)),
currentActiveChannel_(NULL)
{
//    std::stringstream ss;
//    ss<<"EventLoop created "<<this<<" in thread "<<threadId_.second;
    LOG_TRACE<<"EventLoop created "<<this<<" in thread "<<threadId_.second;
//    poller_.reset(new Poller(this));
//    poller_.reset(Poller::newDefaultPoller(this));
    if(t_loopInThisThread)
    {
//        ss.clear();
//        ss<<"Another EventLoop "<<t_loopInThisThread<<" exists in this thread "<<threadId_.second;
        LOG_FATAL<<"Another EventLoop "<<t_loopInThisThread<<" exists in this thread "<<threadId_.second;
    }
    else
    {
        t_loopInThisThread=this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_.second
              << " destructs in thread " << ::syscall(SYS_gettid);
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_= true;
    quit_= false;
    LOG_TRACE<<"EventLoop "<<this<<" start looping";
    while(!quit_)
    {
        activeChanels_.clear();
        pollReturnTime_=poller_->poll(kPollTimeMs,&activeChanels_);
        ++iteration_;
        if(Logger::logLevel()<=Logger::TRACE)
        {
            printActiveChannels();
        }
        eventHandling_=true;
//        for(ChannelList::iterator it=activeChanels_.begin();it!=activeChanels_.end();++it)
//        {
//           //FIXME (*it)->handleEvent();
//        }
        for(const auto &it:activeChanels_)
        {
            currentActiveChannel_=it;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_=NULL;
        eventHandling_= false;
        doPendingFunctors();
    }
    LOG_TRACE<<"EventLoop "<< this<<" stop running";
    looping_= false;
}

void EventLoop::abortNotInLoopThread()
{
//    std::stringstream ss;
    LOG_FATAL<< "EventLoop::abortNotInLoopThread - EventLoop " << this
      << " was created in threadId_ = " << threadId_.second
      << ", current thread id = " <<::syscall(SYS_gettid);
//    LOG_FATAL<<ss.str().c_str();
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
        queueInLoop(std::move(cb));
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
//    for(int i=0;i<functors.size();++i)
//    {
//        functors[i]();
//    }
    for(const auto& functor:functors)
    {
        functor();
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
        //当前channel为正在处理的channel或者根本不在activeChanels_中直接出错,程序终止
        assert(currentActiveChannel_==channel||
        std::find(activeChanels_.begin(),activeChanels_.end(),channel)==activeChanels_.end());
    }
    poller_->removeChannel(channel);
}

size_t EventLoop::queueSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pendingFunctors_.size();
}

void EventLoop::cancel(TimerId timerId)
{
    return  timerQueue_->cancel(timerId);
}

bool EventLoop::hasChannel(Channel *channel)
{
    //只有channel所属的loop才有继续查找的必要
    assert(channel->ownerloop()==this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::printActiveChannels() const
{
    for (const Channel* channel : activeChanels_)
    {
        LOG_TRACE << "{" << channel->reventsToString() << "} ";
    }
}

int EventLoop::createWakeupfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}
