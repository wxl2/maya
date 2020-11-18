//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_EVENTLOOPTHREADPOOL_H
#define MAYA_EVENTLOOPTHREADPOOL_H

#include "base/Types.h"
#include "base/nocopyable.h"

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
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* baseLoop,const string& name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads){numThread_=numThreads;}
    void start(const ThreadInitCallback& cb=ThreadInitCallback());
    EventLoop* getNextLoop();

    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const
    {return started_;}

    const string& name()const
    {return name_;}
private:
    EventLoop* baseLoop_;
    string name_;
    bool started_;
    int numThread_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}//namespace net
}//namespace maya


#endif //MAYA_EVENTLOOPTHREADPOOL_H
