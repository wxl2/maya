set(base_SRCS
        Timestamp.cpp
        LogStream.cpp
        FileUtil.cpp)

add_library(base ${base_SRCS})
target_link_libraries(base pthread)