//
// Created by wxl on 2020/10/16.
//

#ifndef MAYA_BLOCKINGQUEUE_H
#define MAYA_BLOCKINGQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <assert.h>

#include "nocopyable.h"

namespace maya{
    template <typename T>
    class BlockingQueue:nocopyable
    {
    public:
        BlockingQueue():queue_()
        {
        }
        void put(const T& x)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(x);
            notEmpty.notify_one();
        }

        void put(T&& x)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(std::move(x));
            notEmpty.notify_one();
        }

        T take()
        {
            std::unique_lock<std::mutex> lock(mutex_);

            while (queue_.empty())
            {
                notEmpty.wait(lock);
            }

            assert(queue_.empty());
            T front(std::move(queue_.front());
            queue_.pop_front();
            return front;
        }

        int size() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }
    private:
        mutable std::mutex mutex_;
        std::condition_variable notEmpty;
        std::deque<T> queue_;

    };
}//end of maya
#endif //MAYA_BLOCKINGQUEUE_H
