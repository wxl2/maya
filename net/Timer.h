//
// Created by wxl on 2020/10/24.
//

#ifndef MAYA_TIMER_H
#define MAYA_TIMER_H
#include <atomic>
#include "Callbacks.h"
#include "../base/Timestamp.h"
#include "../base/nocopyable.h"

namespace maya{
namespace net{
    class Timer:nocopyable
    {
    public:
        Timer(TimerCallback cb,Timestamp when,double interval)
        :callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval>0.0),
        sequence_(++s_numCreated_)
        {
        }

        void run() const
        {
            callback_();
        }

        Timestamp expiration() const
        {
            return expiration_;
        }

        bool repeat() const{return repeat_;}

        int64_t sequence() const{return sequence_;}

        static int64_t numCreated(){return s_numCreated_;}

        void restart(Timestamp now);
    private:
        const TimerCallback callback_;
        Timestamp expiration_;
        const double interval_;//重复的间隔时间
        const bool repeat_;//是否重复
        const int64_t sequence_;//编号

        static std::atomic<int64_t> s_numCreated_;//用于设置每个定时器的唯一编号
    };
}//namespace net
}//namespace maya

#endif //MAYA_TIMER_H
