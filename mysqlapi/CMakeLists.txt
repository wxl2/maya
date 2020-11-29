#对MySQL API的封装
set(mysqlapi_SRCS
        Field.cpp
        QueryResult.cpp
        DatabaseMysql.cpp)

link_directories(/home/wxl/date/mysql/lib)
include_directories(/home/wxl/date/mysql/include)
add_library(mysqlapi ${mysqlapi_SRCS})
target_link_libraries(mysqlapi mysqlclient base)