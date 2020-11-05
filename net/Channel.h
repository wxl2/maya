//
// Created by wxl on 2020/10/23.
//

#ifndef MAYA_CHANNEL_H
#define MAYA_CHANNEL_H

#include "../base/nocopyable.h"
#include "../base/Timestamp.h"
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
        void handleEvent(Timestamp receiveTime);

        void setReadCallback(const ReadEventCallback& cb)
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
        void enableWriting(){events_|kWriteEvent;update();}
        void disableWriting(){events_&~kWriteEvent;update();}
        void disableAll(){events_=kNoneEvent;update();}
        bool isWriting() const{return events_&kWriteEvent;}


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

        bool eventHandling_;

        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
        EventCallback closeCallback_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_CHANNEL_H
