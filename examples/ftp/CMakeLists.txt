set(ftpserver_SRSC
        server/TcpSession.cpp
        server/FileSession.cpp
        server/FileManager.cpp
        server/FileServer.cpp)

set(clientserver_SRSC
        client/FileClient.cpp)


add_library(ftpserver ${ftpserver_SRSC})
add_executable(ftpServer server/main.cpp)
target_link_libraries(ftpServer ftpserver net utlis)

add_library(ftpclient ${clientserver_SRSC})
add_executable(ftpClient client/main.cpp)
target_link_libraries(ftpClient ftpclient net utlis)