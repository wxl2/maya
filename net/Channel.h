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
        ///处理到来的事件分发
        void handleEvent(Timestamp receiveTime);

        //通常由拥有channel的对象调用
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
        /// 当channel使用智能指针管理时,避免channel被析构
        ///tie此方法是防止Channel类还在执行，上层调用导致
        ///Channel提前释放而出现的异常问题，
        ///tie_中其实保存的是一个TcpConnection对象,channel管理的智能指针一般是std::unique_ptr
        ///只有当拥有std::unique_ptr的对象销毁时std::unique_ptr所管理的对象才会施放
        ///std::unique_ptr 对其持有的堆内存具有唯一拥有权，也就是说引用计数永远是 1，std::unique_ptr 对象销毁时会释放其持有的堆内存
        ///所以只要拥有channel的对象不被析构造,channel就不会被释放,而TcpConnection由于其模糊的生命周期所以使用的是std::share_ptr
        ///来管理其生命周期std::shared_ptr 持有的资源可以在多个 std::shared_ptr 之间共享，每多一个 std::shared_ptr 对资源的引用，
        ///资源引用计数将增加 1，每一个指向该资源的 std::shared_ptr 对象析构时，资源引用计数减 1，最后一个 std::shared_ptr 对象析构时，
        ///发现资源计数为 0，将释放其持有的资源。
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
//
//                              100
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

        //将fd的新增的事件更新到poll或epoll的监听事件当中
        void update();
        ///有保护的进行事件处理,事件分发函数
        void handleEventWithGuard(Timestamp receiveTime);

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop* loop_;
        const int fd_;
        int events_;//保存监听的事件
        int revents_;//区分poll和epoll poll结构体中有revents保存返回发生了的事件,epoll中无次变量
        bool logHup_;//对方描述符挂起是否挂起POLLHUP

        ///in epoll: channel的操作标记add or del or (-1 is new)
        ///in poll: index_为channel在polls数组中的第index_个位置
        int index_;
        ///是否正在处理事件
        bool eventHandling_;
        ///FIXME:参考https://blog.csdn.net/H514434485/article/details/90339554
        ///weak_ptr不增加share_ptr引用计数,但可以通过lock函数返回一个share_ptr从而增加其引用计数,如handEvent()中的tie_.lock()
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
