//
// Created by wxl on 2020/11/25.
//

#include "utils/UUIDGenerator.h"
#include <iostream>

int main()
{
    std::cout<<maya::detail::UUIDGenerator::generate()<<std::endl;
    return 0;
}