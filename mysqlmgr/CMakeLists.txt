set(mysqlmgr_SRCS
        MysqlThrd.cpp
        TaskList.cpp
        MysqlThrdMgr.cpp)

add_library(mysqlmgr ${mysqlmgr_SRCS})
target_link_libraries(mysqlmgr mysqlapi)