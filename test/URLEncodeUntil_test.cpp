//
// Created by wxl on 2020/11/24.
//

#include "utils/URLEncodeUtil.h"
#include <iostream>

using namespace maya;
using namespace maya::detail;

int main()
{
    std::string src="https://zgao.top/golang-nginx-%e5%88%b6%e4%bd%9c%e4%b8%80%e4%b8%aa%e9%9d%99%e6%80%81pornhub%e7%bd%91%e7%ab%99/";//%e4%bd%a0%e5%a5%bd
    std::string str="https://zgao.top/golang-nginx-制作一个静态pornhub网站/";
    std::string dst;
    URLEncodeUtil::encode(str,dst);
    std::cout<<dst;
    return 0;
}