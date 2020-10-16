//
// Created by wxl on 2020/10/14.
//
#include "../base/Logging.h"



int main()
{

    maya::Logger::setLogLevel(maya::Logger::TRACE);
    LOG_TRACE << "trace NYT";
    LOG_DEBUG << "debug NYT";
    LOG_INFO << "Hello NYT";
    LOG_WARN << "World NYT";
    LOG_ERROR << "Error NYT";
    return 0;
}



