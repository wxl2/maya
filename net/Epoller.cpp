//
// Created by wxl on 2020/11/5.
//

#include "Epoller.h"
#include <sys/epoll.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

using namespace maya;
using namespace maya::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

namespace
{
    const int kNew = -1;//新建的channel
    const int kAdded = 1;//在epoll监听列表且在channels_中的channel
    const int kDeleted = 2;//在channels_中但不在epoll监听列表的channel
}

Epoller::Epoller(EventLoop *loop) : Poller(loop)
,epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
events_(kInitEventListSize)
{
    if(epollfd_<0)
    {
        LOG_SYSFATAL<<"Epoller::Epoller";
    }
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

Timestamp Epoller::poll(int timeoutMs, Poller::ChannelList *activeChannels)
{
    LOG_TRACE<<"fd total count"<<channels_.size();
    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno=errno;
    Timestamp now(Timestamp::now());

    if(numEvents>0)
    {
        LOG_TRACE<<numEvents<<" evnets happened";
        fillActiveChannels(numEvents,activeChannels);

        //如果vector满了扩大两倍
        if(implicit_cast<int>(events_.size())==numEvents)
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents==0)
    {
        LOG_TRACE<<"nothing happened";
    }
    else
    {
        if(saveErrno!=EINTR)
        {
            LOG_SYSERR<<"Epoller::poll()";
        }
    }

    return now;
}

void Epoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const
{
    assert(implicit_cast<size_t>(numEvents)<=events_.size());

    for(int i=0;i<numEvents;++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd=channel->fd();
        ChannelMap::const_iterator  it = channels_.find(fd);
        assert(it!=channels_.end());
        assert(it->second==channel);
#endif

        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void Epoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    const int index=channel->index();
    LOG_TRACE<<"fd = "<<channel->fd()<<" evnets = "<<channel->events()<<" index = "<<index;
    //如果channel->index()为kNew(新创建的channel)或者是KDelete(原来有的channel但是监听事件被del了)
    //为这个channel添加事件
    if(index==kNew||index==kDeleted)
    {
        int fd=channel->fd();
        if(index==kNew)
        {
            //如果是新建的channel那么channels_中便没有这个channel
            assert(channels_.find(fd)==channels_.end());
            //将这个channel添加到channels_中
            channels_[fd]=channel;
        }
        else //index ==kDeleted
        {
            //如果是原有的channel那么channels_中便有这个channel
            assert(channels_.find(fd)!=channels_.end());
            //断言原来channels_中的channel是否发生改变,这个函数传进来的是一个channel所以可以是新建的也可以是以前的
            assert(channels_[fd]==channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        //更新监听事件
        int fd=channel->fd();
        (void)fd;
        assert(channels_.find(fd)!=channels_.end());
        assert(channels_[fd]==channel);
        assert(index==kAdded);
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void Epoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    int fd=channel->fd();
    LOG_TRACE<<"fd = "<<fd;
    assert(channels_.find(fd)!=channels_.end());
    assert(channels_[fd]==channel);
    assert(channel->isNoneEvent());
    int index=channel->index();
    assert(index==kAdded||index==kDeleted);
    size_t n=channels_.erase(fd);
    (void )n;
    assert(n==1);//成功返回1

    if(index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);//kAdded表示其还在epoll的监听列表中,将channel从epoll的监听列表中移除
    }
    channel->set_index(kNew);
}

const char *Epoller::operationToString(int op)
{
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}

void Epoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memZero(&event,sizeof(event));
    event.events=channel->events();
    event.data.ptr=channel;
    int fd=channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
    if(::epoll_ctl(epollfd_,operation,fd,&event)<0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
    }
}
