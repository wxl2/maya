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
#include "base/Timestamp.h"
#include "base/nocopyable.h"
#include "base/Logging.h"
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

        /// Loops forever.
        ///
        ///必须在创建EventLoop对象的线程中调用
        ///
        /// Must be called in the same thread as creation of the object.
        void loop();//事件循环

        /// Quits loop.
        ///
        /// This is not 100% thread safe, if you call through a raw pointer,//即裸指针EventLoop* loop;loop->quit()
        /// better to call through shared_ptr<EventLoop> for 100% safety.
        void quit();

        //返回poll调用返回时间
        Timestamp pollReturnTime() const{return pollReturnTime_;}

        int64_t iteration() const{return iteration_;}

        size_t queueSize() const;

        /// Runs callback immediately in the loop thread.
        /// It wakes up the loop, and run the cb.
        /// If in the same loop thread, cb is run within the function.
        /// Safe to call from other threads.
        void runInLoop(Functor cb);

        /// Queues callback in the loop thread.
        /// Runs after finish pooling.
        /// Safe to call from other threads.
        void queueInLoop(Functor cb);

        // timers，时间单位均是微秒
        ///
        /// Runs callback at 'time'.
        /// Safe to call from other threads.
        ///
        TimerId runAt(const Timestamp& time,const TimerCallback& cb);
        ///
        /// Runs callback after @c delay seconds.
        /// Safe to call from other threads.
        ///
        TimerId runAfter(double delay,const TimerCallback& cb);
        ///
        /// Runs callback every @c interval seconds.
        /// Safe to call from other threads.
        ///
        TimerId runEvery(double interval,const TimerCallback& cb);

        ///
        /// Cancels the timer.
        /// Safe to call from other threads.
        ///
        void cancel(TimerId timerId);

        void wakeup();//唤醒IO线程来执行任务
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        ///判断channel是否还在IO线程的监听列表当中
        bool hasChannel(Channel* channel);

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() { return threadId_.first == std::this_thread::get_id(); }
        bool eventHandling() const{return eventHandling_;}

        const int getThreadID()const
        {
            return static_cast<int>(threadId_.second);
        }

        static EventLoop* getEventLoopOfCurrentThread();
    private:
        static int createWakeupfd();
        void abortNotInLoopThread();
        void handleRead();
        void doPendingFunctors();

        void printActiveChannels() const;//for debug

        typedef std::vector<Channel*> ChannelList;

        bool looping_;//bool变量是原子操作
        bool quit_;//atomic
        bool eventHandling_;//用于事件处理
        bool callingPendingFunctors_;
        int64_t iteration_;//loop循环的次数
        const std::pair<std::thread::id,pid_t> threadId_;
        int wakeupFd_;//用于唤醒IO线程
        Timestamp pollReturnTime_;
        mutable std::mutex mutex_;
        //没有在IO线程的时候添加的要调用的函数,
        std::vector<Functor> pendingFunctors_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<Channel> wakeupChannel_;//Channel必须定义在Poller后否则析构时会出错,用于唤醒IO线程的channel,这个channel监听wakeupFd_是否有事件到来,有便会唤醒IO线程
        std::unique_ptr<TimerQueue> timerQueue_;

        ChannelList activeChanels_;
        Channel* currentActiveChannel_;

    };
}//namespace net
}//namespace maya

#endif //MAYA_EVENTLOOP_H
