//
// Created by wxl on 2020/11/5.
//

#ifndef MAYA_EPOLLER_H
#define MAYA_EPOLLER_H
#include "../base/nocopyable.h"
#include "../base/Types.h"
#include "Poller.h"

struct epoll_event;

namespace maya{
namespace net{


    class Epoller: public Poller
    {
    public:
        Epoller(EventLoop* loop);
        ~Epoller() override;

        Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;
        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;

    private:
        //初始化时epoll_event结构体的个数
        static const int kInitEventListSize=16;

        //返回执行的是什么操作,如EPOLL_ADD,DEL
        static const char* operationToString(int op);

        //处理EventLoop传来的activeChannels指针,在其中保存有事件到来的Channel并返回
        void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;

        //调用epoll_ctl改变channel的监听事件
        void update(int operation,Channel* channel);

        typedef std::vector<struct epoll_event> EventList;

        int epollfd_;
        EventList events_;
    };

}//namespace net
}//namespace maya



#endif //MAYA_EPOLLER_H
