//
// Created by wxl on 2020/10/23.
//

#include "Channel.h"
#include "EventLoop.h"
#include <sys/poll.h>

using namespace maya;
using namespace maya::net;

const int Channel::kNoneEvent=0;
const int Channel::kReadEvent=POLLIN|POLLPRI;
const int Channel::kWriteEvent=POLLOUT;
Channel::Channel(EventLoop *loop, int fd)
:loop_(loop),
fd_(fd),
events_(0),
revents_(0),
index_(-1)
{
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    eventHandling_= true;
    if(revents_&POLLNVAL)
    {
        LOG_WARN<<"Channel::handleEvent() POLLNVAL";
    }
    if((revents_&POLLHUP)&&!(revents_&POLLIN))
    {
        LOG_WARN<<"Channel::handleEvent() POLLHUP";
        if(closeCallback_) closeCallback_();
    }
    if(revents_&(POLLERR|POLLNVAL))
    {
        if(errorCallback_) errorCallback_();
    }
    if(revents_&(POLLIN|POLLPRI|POLLHUP))
    {
        //FIXME if(readCallback_) readCallback_();
    }
    if(revents_&POLLOUT)
        if(writeCallback_) writeCallback_();
    eventHandling_= false;
}

Channel::~Channel()
{
    assert(!eventHandling_);
}

