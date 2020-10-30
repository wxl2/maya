//
// Created by wxl on 2020/10/23.
//

#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include "EventLoop.h"

#include <unistd.h>
#include <sys/timerfd.h>
#include <functional>

namespace maya{
namespace net{
namespace detail {
    struct timespec howMuchTimeFromNow(Timestamp when) {
        int64_t microseconds = when.microSecondsSinceEpoch()
                               - Timestamp::now().microSecondsSinceEpoch();
        if (microseconds < 100) {
            microseconds = 100;
        }
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(
                microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
                (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
        return ts;
    }

    void resetTimerfd(int timerfd, Timestamp expiration) {
        // wake up loop by timerfd_settime()
        struct itimerspec newValue;
        struct itimerspec oldValue;
        memZero(&newValue, sizeof newValue);
        memZero(&oldValue, sizeof oldValue);
        newValue.it_value = howMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
        if (ret) {
            LOG_SYSERR << "timerfd_settime()";
        }
    }
}//namespace detail
}//namespace net
}//namespace maya

using namespace maya;
using namespace maya::net;
using namespace maya::net::detail;
TimerQueue::TimerQueue(EventLoop *loop)
:loop_(loop),
timerfd_(::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC)),
timerfdChannel(loop,timerfd_),
timers_()
{
    timerfdChannel.setReadCallback(std::bind(&TimerQueue::handleRead,this));
    timerfdChannel.enableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
    for(const Entry& timer:timers_)
    {
        delete timer.second;
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it=timers_.lower_bound(sentry);

    assert(it==timers_.end()||now<it->first);

    std::copy(timers_.begin(),it,back_inserter(expired));
    timers_.erase(timers_.begin(),it);

    return expired;
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval)
{
    Timer* timer=new Timer(std::move(cb),when,interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this,timer));
    return TimerId(timer,timer->sequence());

}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged=insert(timer);
    if(earliestChanged)
    {
        resetTimerfd(timerfd_,timer->expiration());
    }
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());

}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this,timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size()==activeTimers_.size());
    ActiveTimer timer(timerId.timer_,timerId.sequence_);
    ActiveTimerSet::iterator it=activeTimers_.find(timer);
    if(it!=activeTimers_.end())
    {
        size_t n=timers_.erase(Entry(it->first->expiration(),it->first));
        assert(n==1);(void)n;
        delete it->first;
        activeTimers_.erase(it);
    } else if(callingExpiredTimers_){
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size()==activeTimers_.size());
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
{
    Timestamp nextExpire;
    for(const Entry& it:expired)
    {
        ActiveTimer timer(it.second,it.second->sequence());
        if(it.second->repeat()&&cancelingTimers_.find(timer)==cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    }
    if(!timers_.empty())
    {
        nextExpire=timers_.begin()->second->expiration();
    }

    if(nextExpire.valid())
    {
        resetTimerfd(timerfd_,nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size()==activeTimers_.size());
    bool earliestChanged= false;
    Timestamp when=timer->expiration();
    TimerList::iterator it=timers_.begin();
    if(it==timers_.end()||when<it->first)
    {
        earliestChanged= true;
    }
    {
        std::pair<TimerList::iterator, bool> result
                = timers_.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result
                = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
