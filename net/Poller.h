//
// Created by wxl on 2020/10/23.
//

#ifndef MAYA_POLLER_H
#define MAYA_POLLER_H

#include "base/nocopyable.h"
#include "EventLoop.h"
#include "base/Timestamp.h"
#include <vector>
#include <map>
#include <sys/poll.h>

namespace maya{
namespace net{
    class Channel;
    class Poller:nocopyable
    {
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(EventLoop* loop);
        virtual ~Poller();

        //调用I/O复用函数处理事件到来
        //必须在I/O线程中调用 ==Must be called in the loop thread
        virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels)=0;

        //更新I/O线程中的Channel监听的事件
        //Must be called in the loop thread
        virtual void updateChannel(Channel* channel)=0;

        //当channel没有事件时,将其从I/O线程的监听列表中删除
        //Must be called in the loop thread
        virtual void removeChannel(Channel* channel) = 0;

        //判断监听列表channels_中是否有这个channel
        virtual bool hasChannel(Channel* channel) const;
        void assertInLoopThread()const{ownerLoop_->assertInLoopThread();}

        static Poller* newDefaultPoller(EventLoop* loop);

    protected:
//        void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;

//        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int,Channel*> ChannelMap;

        EventLoop* ownerLoop_;
//        PollFdList pollfds_;
        ChannelMap channels_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_POLLER_H
