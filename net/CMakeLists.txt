set(net_SRCS
        EventLoop.cpp
        Channel.cpp
        Poller.cpp
        TimerQueue.cpp
        Timer.cpp
        SocketsOps.cpp
        InetAddress.cpp
        Buffer.cpp
        Acceptor.cpp
        Socket.cpp
        TcpServer.cpp
        TcpConnection.cpp
        EventLoopThread.cpp
        EventLoopThreadPool.cpp
        Connector.cpp
        Epoller.cpp
        TcpClient.cpp
        Poller.cpp)


add_library(net ${net_SRCS})
target_link_libraries(net base pthread)