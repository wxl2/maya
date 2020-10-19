set(base_SRCS
        Timestamp.cpp
        LogStream.cpp
        FileUtil.cpp
        LogFile.cpp
        Logging.cpp
        CountDownLatch.cpp
        AsyncLogging.cpp
        LogStream.cpp
        ThreadPool.cpp)


add_library(base ${base_SRCS})
target_link_libraries(base pthread)