//
// Created by wxl on 2020/10/18.
//

#ifndef MAYA_THREADPOOL_H
#define MAYA_THREADPOOL_H

#include "nocopyable.h"
#include "Types.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <deque>
#include <vector>
#include <functional>

namespace maya {

    class ThreadPool:nocopyable
    {
        typedef std::function<void()> Task;
    public:
        explicit ThreadPool(const string& name);
        ~ThreadPool();

        void setMaxQueueSize(int maxSize){maxQueueSize=maxSize;}
        void setThreadInitCallBack(const Task& cb){threadInitCallback_=cb;}

        void start(int numThreads);
        void stop();

        size_t queueSize() const;

        void run(Task f);
    private:
        bool isFull() const;
        void runInThread();
        Task take();

        mutable std::mutex mutex_;
        std::condition_variable notFull;
        std::condition_variable notEmpty;
        Task threadInitCallback_;
        std::vector<std::unique_ptr<std::thread>> threads_;
        std::deque<Task> queue_;
        size_t maxQueueSize;
        string name_;
        bool running_;

    };
}

#endif //MAYA_THREADPOOL_H
