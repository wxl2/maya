//
// Created by wxl on 2020/10/23.
//

#ifndef MAYA_TIMERQUEUE_H
#define MAYA_TIMERQUEUE_H

#include "base/nocopyable.h"
#include "base/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

#include <vector>
#include <set>

namespace maya{
namespace net{
    class EventLoop;
    class Channel;
    class Timer;
    class TimerId;

    /// https://www.cnblogs.com/Solstice/archive/2011/02/06/1949555.html
    class TimerQueue:nocopyable
    {
    public:
        explicit TimerQueue(EventLoop* loop);
        ~TimerQueue();

        TimerId addTimer(const TimerCallback& cb,Timestamp when,double interval);

        void cancel(TimerId timerId);
    private:
        typedef std::pair<Timestamp,Timer*> Entry;
        typedef std::set<Entry> TimerList;
        typedef std::pair<Timer*,int64_t> ActiveTimer;
        typedef std::set<ActiveTimer> ActiveTimerSet;

        void addTimerInLoop(Timer* timer);
        void handleRead();


        void cancelInLoop(TimerId timerId);
        std::vector<Entry> getExpired(Timestamp now);//返回到期时间列表
        void reset(const std::vector<Entry>& expired,Timestamp now);

        bool insert(Timer* timer);

        EventLoop* loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        TimerList timers_;

        //for cancel()
        ActiveTimerSet activeTimers_;
        bool callingExpiredTimers_;
        ActiveTimerSet cancelingTimers_;
    };
}//namespace net
}//namespace maya



#endif //MAYA_TIMERQUEUE_H
