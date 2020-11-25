//
// Created by wxl on 2020/11/24.
//

#include "Base64Util.h"

using namespace maya;
using namespace maya::detail;

char Base64Util::Base64Pad = '=';
std::string Base64Util::Base64Table ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char Base64Util::DecodeTbale[] =
{
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
        -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
        -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};
bool Base64Util::encode(const std::string &src, std::string &dst)
{
    if(src.size()==0)
        return false;
    int length=src.length();
    int current=0;
    //按照无符号整形解释,一劳永逸
    const unsigned char* srcLocal=reinterpret_cast<const unsigned char*>(src.c_str());
    while(length>2)
    {
        dst+=Base64Table[srcLocal[current+0]>>2];
        dst+=Base64Table[((srcLocal[current+0]&0x03)<<4)+(srcLocal[current+1]>>4)];
        dst+=Base64Table[((srcLocal[current+1]&0x0f)<<2)+(srcLocal[current+2]>>6)];
        dst+=Base64Table[srcLocal[current+2]&0x3f];
        current+=3;
        length-=3;
    }
    if(length>0)
    {
        dst+=Base64Table[srcLocal[current+0]>>2];
        if(length%3==1)
        {
            dst+=Base64Table[(srcLocal[current+0]&0x03)<<4];
            dst+="==";
        }
        else if(length%3==2)
        {
            dst+=Base64Table[((srcLocal[current+0]&0x03)<<4)+(srcLocal[current+1]>>4)];
            dst+="=";
        }
    }
    return true;
}

bool Base64Util::decode(const std::string &src, std::string &dst)
{
    if(src.size()==0)
        return false;
    int length=src.length();
    int current=0;
    int i=0,bin;
    unsigned char ch;
    const unsigned char* srcLocal=reinterpret_cast<const unsigned char*>(src.c_str());
    while((ch=srcLocal[current++])!='\0'&&length-->0)
    {
        if(ch==Base64Pad)
        {
            /*
            在解码时，4个字符为一组进行一轮字符匹配。
            两个条件：
                1、如果某一轮匹配的第二个是“=”且第三个字符不是“=”，说明这个带解析字符串不合法，直接返回空
                2、如果当前“=”不是第二个字符，且后面的字符只包含空白符，则说明这个这个条件合法，可以继续。
            */
           if((srcLocal[current]!='=') && (i%4==1))
           {
               return false;
           }
            continue;
        }
        ch=DecodeTbale[ch];
        if(ch<0)//过滤非法字符
            continue;
        switch (i%4)
        {
            case 0:
                bin=ch<<2;
                break;
            case 1:
                bin|=ch>>4;
                dst+=bin;
                bin=(ch&0x0f)<<4;
                break;
            case 2:
                bin|=ch>>2;
                dst+=bin;
                bin=(ch&0x03)<<6;
                break;
            case 3:
                bin|=ch;
                dst+=bin;
                break;
        }
        i++;
    }
    return true;
}
