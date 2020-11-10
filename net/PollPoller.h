//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_POLLPOLLER_H
#define MAYA_POLLPOLLER_H

#include "Poller.h"
#include <vector>

struct pollfd;

namespace maya{
namespace net{

    class PollPoller: public Poller
    {
    public:
        PollPoller(EventLoop* loop);
        ~PollPoller() override;

        Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;
        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;
    private:
        //处理EventLoop传来的activeChannels指针,在其中保存有事件到来的Channel并返回
        void fillActiveChannels(int numEvents,ChannelList* activeChannels)const;

        typedef std::vector<pollfd> PollFdList;

        PollFdList pollfds_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_POLLPOLLER_H
