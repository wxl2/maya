//
// Created by wxl on 2020/11/25.
//

#include "StringUtil.h"
#include <string.h>

using namespace maya;
using namespace maya::detail;
void StringUtil::trimLeft(std::string &str, char trimmed)
{
    std::string tmp=str;
    size_t length=str.length();
    for(size_t i=0;i<length;++i)
    {
        if(tmp[i]!=trimmed)
        {
            str=tmp.substr(i);
            break;
        }
    }
}

void StringUtil::trimRigtht(std::string &str, char trimmed)
{
    std::string tmp=str;
    size_t length=str.length();
    for(size_t i = length-1;i>=0;--i)
    {
        if(tmp[i]!=trimmed)
        {
            str=tmp.substr(0,i);
            break;
        }
    }
}

void StringUtil::trim(std::string &str, char trimmed)
{
    trimLeft(str,trimmed);
    trimRigtht(str,trimmed);
    std::string tmp;
    size_t length=str.length();
    for(size_t i=0;i<length;++i)
    {
        if(str[i]!=trimmed)
            tmp+=str[i];
    }
    str=tmp;
}

void StringUtil::split(const std::string &str, std::vector<std::string> &v, const char *delimiter)
{
    if(delimiter==NULL||str.empty())
        return;
    std::string buf(str);
    std::string substr;
    size_t pos=std::string::npos;
    int delimiterlen=strlen(delimiter);
    while (true)
    {
        pos=buf.find(delimiter);
        if(pos!=std::string::npos)
        {
            substr=buf.substr(0,pos);
            if(!substr.empty())
                v.push_back(substr);
            buf=buf.substr(pos+delimiterlen);
        }
        else
        {
            if(!buf.empty())
                v.push_back(buf);
            break;
        }
    }
}

std::string &StringUtil::replace(std::string &str, const std::string &toReplaced, const std::string &newStr)
{
    if(toReplaced.empty()||newStr.empty())
        return str;
    for(std::string::size_type pos=0;pos!=std::string::npos;pos+=newStr.length())
    {
        pos=str.find(toReplaced,pos);
        if(pos!=std::string::npos)
            str.replace(pos,toReplaced.length(),newStr);
        else break;
    }
    return str;
}
