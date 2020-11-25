//
// Created by wxl on 2020/11/25.
// 各种字符集相互转化工具
// 仅实现了UTF8和GB2312的相互转换
// 仅提供转换不提供判断字节流为 什么编码
//

#ifndef MAYA_CHARACTERUTIL_H
#define MAYA_CHARACTERUTIL_H

#include "base/nocopyable.h"
#include <string>
#include <iconv.h>

namespace maya{
namespace detail{

    class CharacterUtil final : nocopyable
    {
    public:
        CharacterUtil(std::string src,std::string dst);//src->dst
        ~CharacterUtil();
        //建议在数据量小时调用 没有考虑传入数据量特别大的情况
        static bool UTF8toGB2312(const std::string& src,std::string& dst);
        static bool GB2312toUTF8(const std::string& src,std::string& dst);
        //数据量大时调用,一步到位版本,不建议一次性调用,可以多次调用,调用示例见CharacterUtil_test.cpp
        bool UTF8toGB2312(const char* src,size_t srclen,char** dst,size_t* dstlen);
        bool GB2312toUTF8(const char* src,size_t srclen,char** dst,size_t* dstlen);

        //一步到位版本
        size_t convert(const char** src,size_t dstlen);
    private:
        void ReallocBuffer();
        std::string src_;
        std::string dst_;
        iconv_t iconv_;
        char *buffer_;
        size_t bufferLen_;
    };
}//namespace detail
}//namespace maya


#endif //MAYA_CHARACTERUTIL_H
