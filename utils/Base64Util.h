//
// base64编解码工具
// 参考
// https://baike.baidu.com/item/base64/8545775?fr=aladdin
// https://www.cnblogs.com/lrxing/p/5535601.html
// Created by wxl on 2020/11/24.
//

#ifndef MAYA_BASE64UTIL_H
#define MAYA_BASE64UTIL_H
#include "base/nocopyable.h"
#include <string>
namespace maya{
namespace detail{
    class Base64Util final :nocopyable
    {
    public:
        //编码
        static bool encode(const std::string& src,std::string& dst);
        //解码
        static bool decode(const std::string& src,std::string& dst);

    private:
        static std::string Base64Table;//base64编码表
        static char DecodeTbale[256];//解码表
        static char Base64Pad;//base64末尾分割符号'='

    };
}//namespace detail
}//namespace maya


#endif //MAYA_BASE64UTIL_H
