//
// Created by wxl on 2020/11/5.
//

#include "PollPoller.h"
using namespace maya;
using namespace maya::net;
PollPoller::PollPoller(maya::net::EventLoop *loop) : Poller(loop)
{
}

PollPoller::~PollPoller() =default;
Timestamp PollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels)
{
    int numEvents=::poll(&*pollfds_.begin(),pollfds_.size(),timeoutMs);
    int saveErrno=errno;
    Timestamp now(Timestamp::now());
    if(numEvents>0)
    {
        LOG_TRACE<<numEvents<<" events happened";
        fillActiveChannels(numEvents,activeChannels);
    }
    else if(numEvents==0)
    {
        LOG_TRACE<<"nothing  happened";
    }
    else
    {
        if(saveErrno!=EINTR)
        {
            errno=saveErrno;
            LOG_SYSERR<<"PollPoller::poll()";
        }
    }
    return now;
}

void PollPoller::updateChannel(Channel *channel)
{
    //除了能更新channel监听的事件,还能增加事件
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->fd() < 0) {
        //如果这个channel不在监听列表内
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        //将其添加进监听列表
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        //channel在监听列表内
        assert(channels_.find(channel->fd())!=channels_.end());
        assert(channels_[channel->fd()]==channel);
        int idx=channel->index();
        assert(0<=idx&&idx<static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd==channel->fd()||pfd.fd==-channel->fd()-1);
        //如果是重新监听,文件描述符可能不是之前的文件描述符
        pfd.fd=channel->fd();
        //更新事件
        pfd.events=static_cast<short>(channel->events());
        if(channel->isNoneEvent())
        {
            //将这个channel从监听列表中移除,poll不监听负值文件描述符
            pfd.fd=-channel->fd()-1;
        }
    }
}

void PollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    LOG_TRACE<<"fd = "<<channel->fd();
    assert(channels_.find(channel->fd())!=channels_.end());
    assert(channels_[channel->fd()]==channel);
    assert(channel->isNoneEvent());
    int idx=channel->index();
    assert(0<=idx&&idx<static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd=pollfds_[idx];
    (void)pfd;
    assert(pfd.fd==-channel->fd()&&pfd.events==channel->events());
    size_t n =channels_.erase(channel->fd());
    assert(n==1);
    (void)n;
    if(implicit_cast<size_t>(idx)==pollfds_.size()-1)
    {
        //如果是最后一个直接删除
        pollfds_.pop_back();
    }
    else
    {
        int channelAtEnd=pollfds_.back().fd;
        //如果不是最后一个,将当前位置的channel与最后一个channel交换,再删除
        iter_swap(pollfds_.begin()+idx,pollfds_.end()-1);
        if(channelAtEnd<0)
        {
            channelAtEnd=-channelAtEnd-1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}

void PollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const
{
    for(PollFdList::const_iterator it=pollfds_.begin();it!=pollfds_.end()&&numEvents>0;++it)
    {
        if(it->revents>0)
        {
            --numEvents;
            //判断这个channel是否在监听列表当中,若不是直接出错,终止程序
            ChannelMap::const_iterator ch=channels_.find(it->fd);
            assert(ch!=channels_.end());
            Channel* channel=ch->second;
            assert(channel->fd()==it->fd);
            channel->set_revents(it->revents);
            activeChannels->push_back(channel);
        }
    }
}
