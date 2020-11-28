#json-version jsoncpp-1.9.4

set(json_SRSC
        json_reader.cpp
        json_value.cpp
        json_writer.cpp)
add_library(json ${json_SRSC})
