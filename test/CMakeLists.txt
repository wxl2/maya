add_executable(LogStream_test LogStream_test.cpp)
target_link_libraries(LogStream_test base)

add_executable(AppendFIle_test AppendFIle_test.cpp)
target_link_libraries(AppendFIle_test base)

add_executable(LogFile_test LogFile_test.cpp)
target_link_libraries(LogFile_test base)

add_executable(Logging_test Logging_test.cpp)
target_link_libraries(Logging_test base)

add_executable(AsyncLogging_test AsyncLogging_test.cpp)
target_link_libraries(AsyncLogging_test base)

add_executable(CircularList_test CircularList_test.cpp)
target_link_libraries(CircularList_test base)

add_executable(ThreadPool_test ThreadPool_test.cpp)
target_link_libraries(ThreadPool_test base)

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test base net)

add_executable(Acceptor_test Acceptor_test.cpp)
target_link_libraries(Acceptor_test  base net)

add_executable(Base64Util_test Base64Util_test.cpp)
target_link_libraries(Base64Util_test utlis)

add_executable(URLEncodeUntil_test URLEncodeUntil_test.cpp)
target_link_libraries(URLEncodeUntil_test utlis)

add_executable(CharacterUtil_test CharacterUtil_test.cpp)
target_link_libraries(CharacterUtil_test utlis)

add_executable(StringUntil_test StringUntil_test.cpp)
target_link_libraries(StringUntil_test utlis)

add_executable(UUIDGenerator_test UUIDGenerator_test.cpp)
target_link_libraries(UUIDGenerator_test utlis)

add_executable(MD5_test MD5_test.cpp)
target_link_libraries(MD5_test utlis)

add_executable(DaemonRun_test DaemonRun_test.cpp)
target_link_libraries(DaemonRun_test utlis)

add_executable(ProtocolStream_test ProtocolStream_test.cpp)
target_link_libraries(ProtocolStream_test utlis)

add_executable(json_test json_test.cpp)
target_link_libraries(json_test json)

add_executable(mysqlapi_test mysqlapi_test.cpp)
target_link_libraries(mysqlapi_test mysqlapi)