cmake_minimum_required(VERSION 3.17)
project(maya)


set(CMAKE_CXX_STANDARD 14)
link_directories(/home/wxl/date/mysql/lib)
include_directories(/home/wxl/date/mysql/include)
aux_source_directory(. DIRSRCS)
include_directories(./ ./base ./test ./net)
add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(json)
add_subdirectory(utils)
add_subdirectory(mysqlapi)
add_subdirectory(mysqlmgr)


add_executable(maya main.cpp)
target_link_libraries(maya base net)