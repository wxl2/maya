//
// Created by wxl on 2020/10/22.
//

#ifndef MAYA_EVENTLOOP_H
#define MAYA_EVENTLOOP_H

#include <assert.h>
#include <vector>
#include <memory>
#include <thread>
#include <functional>
#include <mutex>
#include "../base/Timestamp.h"
#include "../base/nocopyable.h"
#include "../base/Logging.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "Callbacks.h"

namespace maya{
namespace net {
    class Channel;
    class Poller;
    class EventLoop : nocopyable {
    public:
        typedef std::function<void()> Functor;
        EventLoop();

        ~EventLoop();

        TimerId runAt(const Timestamp& time,const TimerCallback& cb);
        TimerId runAfter(double delay,const TimerCallback& cb);
        TimerId runEvery(double interval,const TimerCallback& cb);

        void loop();//事件循环
        void quit();

        void runInLoop(Functor cb);

        void queueInLoop(Functor cb);
        void wakeup();//唤醒IO线程来执行任务
        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() { return threadId_.first == std::this_thread::get_id(); }

        static EventLoop* getEventLoopOfCurrentThread();
    private:
        void abortNotInLoopThread();
        void handleRead();
        void doPendingFunctors();


        typedef std::vector<Channel*> ChannelList;

        bool looping_;//bool变量是原子操作
        bool quit_;//atomic
        bool callingPendingFunctors_;
        int wakeupFd_;//用于唤醒IO线程
        std::unique_ptr<Channel> wakeupChannel_;
        std::mutex mutex_;
        std::vector<Functor> pendingFunctors_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        ChannelList activeChanels_;
        const std::pair<std::thread::id,pid_t> threadId_;
    };
}//namespace net
}//namespace maya

#endif //MAYA_EVENTLOOP_H
