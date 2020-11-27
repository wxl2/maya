set(http_SRCS
        HttpServer.cpp
        HttpResponse.cpp
        HttpContext.cpp
        FileDown.cpp)

add_library(http ${http_SRCS})
target_link_libraries(http net)

add_executable(http_test main.cpp)
target_link_libraries(http_test http utlis)