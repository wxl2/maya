set(utils_SRSC
        Base64Util.cpp
        URLEncodeUtil.cpp
        CharacterUtil.cpp
        StringUtil.cpp
        UUIDGenerator.cpp
        MD5.cpp
        DaemonRun.cpp
        ProtocolStream.cpp
        ConfigFileReader.cpp)

add_library(utlis ${utils_SRSC})
target_link_libraries(utlis uuid)