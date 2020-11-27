//
// Created by wxl on 2020/11/25.
//

#ifndef MAYA_STRINGUTIL_H
#define MAYA_STRINGUTIL_H

#include "base/nocopyable.h"
#include <string>
#include <vector>
namespace maya{
namespace detail {
    class StringUtil final :nocopyable
    {
    private:
        StringUtil()=delete;
        ~StringUtil()=delete;

    public:
        //分割字符串,默认为','
        static void split(const std::string& str,std::vector<std::string>&v,const char* delimiter=",");

        //将一个字符串内的toReplaced串替换成newStr
        static std::string& replace(std::string& str, const std::string& toReplaced, const std::string& newStr);

        //去除一个字符串开头的一个字符,默认为空格 eg:###aaa trimmed=#调用之后->aaa
        static  void trimLeft(std::string& str,char trimmed=' ');

        //去除一个字符串从结尾开始的一个字符,默认为空格 eg:aaa## trimmed=#调用之后->aaa
        static  void trimRigtht(std::string& str,char trimmed=' ');

        //去除一个字符串内的所有trimmed字符,默认为空格 eg:#aa#a## trimmed=#调用之后->aaa
        static  void trim(std::string& str,char trimmed=' ');
    };
}//namespace detail
}//namespace maya


#endif //MAYA_STRINGUTIL_H
