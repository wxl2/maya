//
// Created by wxl on 2020/11/25.
// 使用uuid库生成uuid
//

#include "UUIDGenerator.h"
#include <uuid//uuid.h>

using namespace maya::detail;

std::string UUIDGenerator::generate()
{
    uuid_t uuid;
    char str[40]={0};

    uuid_generate(uuid);
    uuid_unparse(uuid,str);
    return std::string(str,36);
}
