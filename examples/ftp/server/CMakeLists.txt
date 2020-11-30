set(ftpserver_SRSC
        TcpSession.cpp
        FileSession.cpp
        FileManager.cpp)

add_library(ftpserver ${ftpserver_SRSC})
target_link_libraries(ftpserver net)