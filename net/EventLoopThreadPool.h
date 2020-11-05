//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_EVENTLOOPTHREADPOOL_H
#define MAYA_EVENTLOOPTHREADPOOL_H

#include "../base/Types.h"
#include "../base/nocopyable.h"

#include <functional>
#include <memory>
#include <vector>


namespace maya{
namespace net{
    class EventLoop;
    class EventLoopThread;
class EventLoopThreadPool:nocopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads){numThread_=numThreads;}
    void start();
    EventLoop* getNextLoop();
private:
    EventLoop* loop_;
    bool started_;
    int numThread_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}//namespace net
}//namespace maya


#endif //MAYA_EVENTLOOPTHREADPOOL_H
