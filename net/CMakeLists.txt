set(net_SRCS
        EventLoop.cpp
        Channel.cpp
        Poller.cpp
        TimerQueue.cpp
        Timer.cpp
        SocketsOps.cpp)

add_library(net ${net_SRCS})
target_link_libraries(net base pthread)