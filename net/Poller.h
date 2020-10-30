//
// Created by wxl on 2020/10/23.
//

#ifndef MAYA_POLLER_H
#define MAYA_POLLER_H

#include "../base/nocopyable.h"
#include "EventLoop.h"
#include "../base/Timestamp.h"
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
        ~Poller();

        Timestamp poll(int timeoutMs,ChannelList* activeChannels);

        void updateChannel(Channel* channel);

        void assertInLoopThread(){ownerLoop_->assertInLoopThread();}

    private:
        void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int,Channel*> ChannelMap;

        EventLoop* ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_POLLER_H
