//
// Created by wxl on 2020/11/24.
// URL编解码工具
// https://en.wikipedia.org/wiki/Percent-encoding
//

#ifndef MAYA_URLENCODEUTIL_H
#define MAYA_URLENCODEUTIL_H

#include "base/nocopyable.h"
#include <string>

namespace maya{
namespace detail{
    class URLEncodeUtil final :nocopyable
    {
    public:
        //编码
        static bool encode(const std::string& src,std::string& dst);
        //解码
        static bool decode(const std::string& src,std::string& dst);

    private:
        //用于把16进制的字母表示转换为16进制数字
        static char HexToIntTbale[103];
    };
}//namespace detail
}//namespace maya

#endif //MAYA_URLENCODEUTIL_H
