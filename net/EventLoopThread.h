//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_EVENTLOOPTHREAD_H
#define MAYA_EVENTLOOPTHREAD_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <memory>
#include "base/Types.h"
#include "base/nocopyable.h"
namespace maya{
namespace net{

    class EventLoop;
    class EventLoopThread:nocopyable
    {
    public:
        typedef std::function<void(EventLoop*)> ThreadInitCallback;
        explicit EventLoopThread(const ThreadInitCallback& cb=ThreadInitCallback(),const std::string& name="");
        ~EventLoopThread();
        EventLoop* startLoop();
        void stopLoop();
    private:
        void threadFunc();

        EventLoop* loop_;
        bool exiting_;
        std::unique_ptr<std::thread> thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
        ThreadInitCallback  callback_;
    };

}//namespace net
}//namespace maya

#endif //MAYA_EVENTLOOPTHREAD_H
