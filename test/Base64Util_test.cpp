//
// Created by wxl on 2020/11/24.
//

#include "utils/Base64Util.h"
#include <iostream>


int main()
{
    std::string src="这是一条测试数据：http://img.v.cmcm.com/7dd6e6d7-0c2d-5a58-a072-71f828b94cbc_crop_216x150.jpg";
    std::string dst;
    maya::detail::Base64Util::encode(src,dst);
    std::cout<<dst<<std::endl;
    std::string str;
    maya::detail::Base64Util::decode(dst,str);
    std::cout<<str;
    return 0;
}