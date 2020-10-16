//
// Created by wxl on 2020/10/14.
//

#ifndef MAYA_ASYNCLOGGING_H
#define MAYA_ASYNCLOGGING_H
#include "nocopyable.h"
#include "LogStream.h"
#include "CountDownLatch.h"
#include "Types.h"

#include <mutex>
#include <condition_variable>
#include <thread>

#include <memory>
#include <atomic>
#include <vector>
#include <functional>


namespace maya {

    class AsyncLogging:nocopyable
    {
    public:
        AsyncLogging(const string& basename,
                     off_t rollSize,
                     int flushInterval=3);

        ~AsyncLogging()
        {
            if (running_)
            {
                stop();
            }
        }

        void append(const char* logline, int len);

        void start()
        {
            running_ = true;
            thread_.reset(new std::thread(std::bind(&AsyncLogging::threadFunc,this)));
            latch_.wait();
        }

        void stop()
        {
            running_ = false;
            cond_.notify_one();
            if(thread_->joinable())
                thread_->join();
        }
    private:
        void threadFunc();

        typedef maya::detail::FixedBuffer<detail::kLargeBuffer> Buffer;
        typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
        typedef BufferVector::value_type BufferPtr;

        int flushInterval_;
        std::atomic<bool> running_;
        const string basename_;
        const off_t rollSize_;
        static std::unique_ptr<std::thread> thread_;
        static std::mutex mutex_;
        static std::condition_variable cond_;
        maya::CountDownLatch latch_;
        BufferPtr currentBuffer_;
        BufferPtr nextBuffer_;
        BufferVector buffers_;
    };
}//namspace maya;

#endif //MAYA_ASYNCLOGGING_H
