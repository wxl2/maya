//
// Created by wxl on 2020/10/18.
//

#include "ThreadPool.h"

using namespace maya;

ThreadPool::ThreadPool(const string& name)
:maxQueueSize(0),
name_(name),
running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if(running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(queue_.empty());
    running_= true;
    threads_.reserve(numThreads);
    for(int i=0;i<numThreads;++i)
    {
        threads_.emplace_back(new std::thread(std::bind(&ThreadPool::runInThread,this)));
    }
    if(numThreads==0&&threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        notFull.notify_all();
        notEmpty.notify_all();
    }
    for (auto& thr : threads_)
    {
        if(thr->joinable())
            thr->join();
    }
}

size_t ThreadPool::queueSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(ThreadPool::Task f)
{
    if(threads_.empty())
    {
        take();
    }
    else
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (isFull()&&running_)
        {
            notFull.wait(lock);
        }
        if(!running_)return;
        assert(!isFull());
        queue_.push_back(std::move(f));
        notEmpty.notify_one();
    }
}

bool ThreadPool::isFull() const
{
//    std::lock_guard<std::mutex> lock(mutex_);
    return maxQueueSize>0&&queue_.size()>=maxQueueSize;
}

void ThreadPool::runInThread()
{
    try
    {
        if(threadInitCallback_)
        {
         threadInitCallback_();
        }
        while (running_)
        {
            Task task(take());
            if(task)
            {
                 task();
            }
        }
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw;
    }
}

ThreadPool::Task ThreadPool::take()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.empty()&&running_)
    {
        notEmpty.wait(lock);
    }
    Task task;
    if(!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize>0)
        {
            notFull.notify_one();
        }
    }
    return task;
}


