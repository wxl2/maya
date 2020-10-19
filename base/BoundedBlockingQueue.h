//
// Created by wxl on 2020/10/16.
//

#ifndef MAYA_BOUNDEDBLOCKINGQUEUE_H
#define MAYA_BOUNDEDBLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <assert.h>
#include "nocopyable.h"
#include "CircularList.h"

namespace maya{

    template<typename T>
    class BoundedBlockingQueue:nocopyable
    {
    public:
        BoundedBlockingQueue(int maxSize):queue_(maxSize){}

        void put(const T& x)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(queue_.full())
            {
                notFull.wait(lock);
            }
            assert(!queue_.full());
            queue_.push_back(x);
            notEmpty.notify_one();
        }

        void put(T&& x)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.full())
            {
                notFull.wait(lock);
            }
            assert(!queue_.full());
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
            assert(!queue_.empty());
            T front(std::move(queue_.front()));
            queue_.pop_front();
            notFull.notify_one();
            return front;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        bool full() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.full();
        }

        size_t size() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return static_cast<size_t>(queue_.size());
        }

        size_t capacity() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return static_cast<size_t>(queue_.capacity());
        }
    private:
        mutable std::mutex mutex_;
        std::condition_variable notFull;
        std::condition_variable notEmpty;
        CricularList<T> queue_;
    };
}//namespace maya
#endif //MAYA_BOUNDEDBLOCKINGQUEUE_H
