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