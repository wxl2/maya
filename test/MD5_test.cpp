//
// Created by wxl on 2020/11/25.
//

#include "utils/MD5.h"
#include <iostream>

int main()
{
    maya::detail::MD5 md5("aaaa");
    std::cout<<md5.toString()<<std::endl;
    return 0;
}