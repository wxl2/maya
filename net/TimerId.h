//
// Created by wxl on 2020/10/24.
//

#ifndef MAYA_TIMERID_H
#define MAYA_TIMERID_H

#include "base/copyable.h"
#include <stdint.h>
#include <stdio.h>

namespace maya{
namespace net{
    class Timer;

    class TimerId : public maya::copyable
    {
    public:
        TimerId()
                : timer_(NULL),
                  sequence_(0)
        {
        }

        TimerId(Timer* timer, int64_t seq)
                : timer_(timer),
                  sequence_(seq)
        {
        }


        friend class TimerQueue;

    private:
        Timer* timer_;
        int64_t sequence_;
    };
}//namespace net
}//namespace maya

#endif //MAYA_TIMERID_H
