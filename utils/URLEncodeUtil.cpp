//
// Created by wxl on 2020/11/24.
//

#include "URLEncodeUtil.h"
using namespace maya;
using namespace maya::detail;

char URLEncodeUtil::HexToIntTbale[]={
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, 0, 1,
         2, 3, 4, 5, 6, 7, 8, 9,-1,-1,
        -1,-1,-1,-1,-1,10,11,12,13,14,
        15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,10,11,12,
        13,14,15
};
bool URLEncodeUtil::encode(const std::string &src, std::string &dst)
{
    if(src.length()<0)
        return false;
    const char hex[]="0123456789ABCDEF";
    size_t size=src.size();//size()和length()没有区别
    //URL中绝不编码的字符-	_	.   ~;
    //因为百分比字符（%）用作百分比编码八位字节的指示符，所以必须将该百分比八位字节编码%25为该八位字节才能用作URI中的数据。
    //将一个数分为两个16进制的形式保存如# accii码为35 URL 编码为%23 35/16=2 35%16=3 /16和>>4相同
    //如果是数字和字母也不进行URL编码
    for(size_t i=0;i<size;++i)
    {
        //一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]
        if(isalnum((unsigned char)src[i])||
                (src[i]=='-')||(src[i]==':')||(src[i]=='$')||(src[i]=='+')||(src[i]=='&')||
                (src[i]=='_')||(src[i]=='(')||(src[i]=='!')||(src[i]=='*')||(src[i]=='/')||
                (src[i]=='.')||(src[i]==')')||(src[i]=='\'')||(src[i]==',')||(src[i]==';')||
                (src[i]=='~')||(src[i]=='=')||(src[i]=='?')||(src[i]=='@'))
            dst+=src[i];
        else
        {
            dst+='%';
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst+=hex[c>>4];
            dst+=hex[c%16];
        }
    }
    return true;
}

bool URLEncodeUtil::decode(const std::string &src, std::string &dst)
{
    int i=sizeof(HexToIntTbale);
    if(src.length()<0)
        return false;
    size_t size=src.size();
    char buf[]="你好";
    for(size_t i=0;i<src.size();++i)
    {
        if(src[i]=='%')
        {
            if(i+2>size)
                return false;
            unsigned char high=HexToIntTbale[(unsigned char)src[++i]];
            unsigned char low=HexToIntTbale[(unsigned char)src[++i]];
            char num=high*16+low;
            dst+=high*16+low;
        }
        else
            dst+=src[i];
    }
    return true;
}
