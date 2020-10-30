//
// Created by wxl on 2020/10/24.
//

#include "Timer.h"

using namespace maya;
using namespace maya::net;

::std::atomic<int64_t> Timer::s_numCreated_;
void Timer::restart(maya::Timestamp now)
{
    if(repeat_)
    {
        expiration_=addTime(now,interval_);
    }
    else
    {
        expiration_=Timestamp::invalid();
    }
}
