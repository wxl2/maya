//
// Created by wxl on 2020/10/14.
//

#ifndef MAYA_COUNTDOWNLATCH_H
#define MAYA_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>

namespace maya{
class CountDownLatch
{
public:

    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable std::mutex      mutex_;
    std::condition_variable condition_;
    int                     count_;
};

}//namespace maya
#endif //MAYA_COUNTDOWNLATCH_H
