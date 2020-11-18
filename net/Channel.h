//
// Created by wxl on 2020/10/23.
//

#ifndef MAYA_CHANNEL_H
#define MAYA_CHANNEL_H

#include "base/nocopyable.h"
#include "base/Timestamp.h"
#include <functional>
#include <memory>

namespace maya{
namespace net{

    class EventLoop;
    class Channel:nocopyable
    {
    public:
        typedef std::function<void()> EventCallback;
        typedef std::function<void(Timestamp)> ReadEventCallback;

        Channel(EventLoop *loop,int fd);

        ~Channel();
        ///处理到来的事件
        void handleEvent(Timestamp receiveTime);

        void setReadCallback(const ReadEventCallback& cb)
        {readCallback_=cb;}
        void setWriteCallback(const EventCallback& cb)
        {writeCallback_=cb;}
        void setErrorCallback(const EventCallback& cb)
        {errorCallback_=cb;}
        void setCloseCallback(const EventCallback& cb)
        {closeCallback_=cb;}

        /// Tie this channel to the owner object managed by shared_ptr,
        /// prevent the owner object being destroyed in handleEvent.
        void tie(const std::shared_ptr<void>&);

        int fd() const{return fd_;}
        int events() const{return events_;}
        void set_revents(int revt){revents_=revt;}//use by Pollpollers
//        void add_revents(int revt){revents_|=revt;}//use by Pollpollers
        bool isNoneEvent() const{return events_==kNoneEvent;}

        //监听读事件，在events_中添加读事件，调用update将其添加到监听队列
        void enableReading() { events_ |= kReadEvent; update(); }
        void disableReading() { events_ &= ~kReadEvent; update(); }
        void enableWriting() { events_ |= kWriteEvent; update(); }
        void disableWriting() { events_ &= ~kWriteEvent; update(); }
        void disableAll(){events_=kNoneEvent;update();}
        bool isWriting() const{return events_&kWriteEvent;}
        bool isReading() const{return events_&kReadEvent;}


        //for Pollpoller
        int index(){return index_;}
        void set_index(int idx){index_=idx;}

        //for debug
        string reventsToString()const;
        string eventsToString()const;

        void doNotLogHup(){logHup_= false;}

        //将当前channel从IO线程的监听列表移除
        void remove();
        EventLoop* ownerloop(){return loop_;}

    private:
        static string eventsToString(int fd,int ev);

        void update();
        ///有保护的进行事件处理
        void handleEventWithGuard(Timestamp receiveTime);

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop* loop_;
        const int fd_;
        int events_;
        int revents_;//区分poll和epoll poll结构体中有revents保存返回发生了的事件,epoll中无次变量
        bool logHup_;//对方描述符挂起是否挂起POLLHUP

        ///in epoll: channel的操作标记add or del or (-1 is new)
        ///in poll: index_为channel在polls数组中的第index_个位置
        int index_;
        ///是否正在处理事件
        bool eventHandling_;
        std::weak_ptr<void> tie_;
        bool tied_;
        ///是否已经加入到IO线程进行监听
        bool addedToLoop_;

        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
        EventCallback closeCallback_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_CHANNEL_H
