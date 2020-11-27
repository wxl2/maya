//
// Created by wxl on 2020/11/25.
//

#include "utils/StringUtil.h"
#include <iostream>
using namespace maya::detail;
int main()
{
    std::string str="aaa bbb  ccc ####";
    std::cout<<str<<std::endl;
    StringUtil::trimLeft(str);
    std::cout<<str<<std::endl;
    StringUtil::trimRigtht(str,'#');
    std::cout<<str<<std::endl;
    StringUtil::trim(str);
    std::cout<<str<<std::endl;
    std::vector<std::string> vec;
    str="aaa bbb  ccc aaa ####";
    StringUtil::split(str,vec," ");
    for(auto it:vec)
        std::cout<<it<<std::endl;
    str=StringUtil::replace(str,"aaa","bbb");

    std::cout<<str<<std::endl;
    return 0;
}