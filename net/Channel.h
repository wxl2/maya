//
// Created by wxl on 2020/10/23.
//

#ifndef MAYA_CHANNEL_H
#define MAYA_CHANNEL_H

#include "../base/nocopyable.h"
#include <functional>
#include <memory>

namespace maya{
namespace net{

    class EventLoop;
    class Channel:nocopyable
    {
    public:
        typedef std::function<void()> EventCallback;

        Channel(EventLoop *loop,int fd);

        void handleEvent();

        void setReadCallback(const EventCallback& cb)
        {readCallback_=cb;}
        void setWriteCallback(const EventCallback& cb)
        {writeCallback_=cb;}
        void setErrorCallback(const EventCallback& cb)
        {errorCallback_=cb;}

        int fd() const{return fd_;}
        int events() const{return events_;}
        void set_revents(int revt){revents_=revt;}
        bool isNoneEvent() const{return events_==kNoneEvent;}

        //监听读事件，在events_中添加读事件，调用update将其添加到监听队列
        void enableReading(){events_|=kReadEvent;update();}


        int index(){return index_;}
        void set_index(int idx){index_=idx;}

        EventLoop* ownerloop(){return loop_;}

    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop* loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_CHANNEL_H
