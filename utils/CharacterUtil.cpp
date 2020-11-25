//
// Created by wxl on 2020/11/25.
//

#include "CharacterUtil.h"
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <climits>
#include <cstring>
#include <exception>

using namespace maya;
using namespace maya::detail;

bool CharacterUtil::UTF8toGB2312(const std::string &src, std::string& dst)
{
    if(src.length()<0)
        return false;
    size_t dstlen=0;
    char* outptr= NULL;
    CharacterUtil tmp("utf-8","gb2312");
    if(!tmp.UTF8toGB2312(src.c_str(),src.length(),&outptr,&dstlen))
        return false;
    if(outptr!=NULL)
        dst.append(outptr,dstlen);
    return true;
}

bool CharacterUtil::GB2312toUTF8(const std::string &src, std::string& dst)
{
    if(src.length()<0)
        return false;
    size_t dstlen=0;
    char* outptr= NULL;
    CharacterUtil tmp("gb2312","utf-8");//utf-8->gb2312
    if(!tmp.GB2312toUTF8(src.c_str(),src.length(),&outptr,&dstlen))
        return false;
    if(outptr!=NULL)
        dst.append(outptr,dstlen);
    return true;
}

bool CharacterUtil::UTF8toGB2312(const char *src, size_t srclen, char **dst, size_t *dstlen)
{
    size_t len = convert(&src,srclen);
    if(len==(size_t)-1)
        return false;
    *dst=buffer_;
    *dstlen=len;
    return true;
}

bool CharacterUtil::GB2312toUTF8(const char *src, size_t srclen, char **dst, size_t *dstlen)
{
    size_t len = convert(&src,srclen);
    if(len==(size_t)-1)
        return false;
    *dst=buffer_;
    *dstlen=len;
    return true;
}

//存在一个bug gb2312使用两个字符编码一个汉字,当一个汉字被截断时无法转换,比如  你好的gb2312编码,0xc4,0xe3,0xba,0xc3
//当读入的src的长度为2时,恰好前面有一个空格,读入内容就为0x20,0xc4,便无法正确转码,这里的做法是忽略
size_t CharacterUtil::convert(const char **src,size_t len)
{
    //一旦不满足这些条件,说明出错,程序没有必要继续运行
    assert(buffer_!=NULL&&(iconv_!=reinterpret_cast<iconv_t>(-1)));
    assert(src!=NULL&&*src!=NULL&&len>0);

    char *dstBuffer=buffer_;
    size_t srclen=len;
    size_t dstlen=bufferLen_;//bufferLen_;
    char *srcLocal=new char[len];
    if(srcLocal==NULL)
        return (size_t)-1;
    memcpy(srcLocal,*src,len);
    size_t ret=iconv(iconv_,&srcLocal,&srclen,&dstBuffer,&dstlen);
    while(ret==ULONG_MAX)
    {
        if(errno==E2BIG)//缓冲区不足时
        {
            size_t oldLength=bufferLen_;
            ReallocBuffer();
            dstBuffer=buffer_+(oldLength-dstlen);
            dstlen=bufferLen_-(oldLength-dstlen);
            ret=iconv(iconv_,&srcLocal,&srclen,&dstBuffer,&dstlen);
        }
        else if(errno==EILSEQ||errno==EINVAL)
        {
            printf("Characters that cannot be converted\n");
            return (size_t)-1;
        }
        else
        {
            perror("iconv: ");
            return (size_t)-1;
        }
    }
    size_t endFlag=bufferLen_-dstlen;
    if(dstlen==0)
        ReallocBuffer();
    buffer_[endFlag]=0;//在结尾处填上0
    return endFlag;
}

CharacterUtil::CharacterUtil(std::string src, std::string dst)
:src_(src),dst_(dst),iconv_(reinterpret_cast<iconv_t>(-1)),buffer_(NULL),bufferLen_(BUFSIZ)
// //IGNORE这个参数，表示忽略不能转换的字符
{
    if(src_.empty()||dst.empty())
        throw std::exception();
    buffer_=(char*)malloc(bufferLen_);
    if(buffer_==NULL)
        throw std::exception();
    dst+="//IGNORE";
    iconv_=iconv_open(dst.c_str(),src_.c_str());
    if(iconv_==reinterpret_cast<iconv_t>(-1))
        throw std::exception();
}

CharacterUtil::~CharacterUtil()
{
    if(iconv_!=reinterpret_cast<iconv_t>(-1))
        iconv_close(iconv_);
    if(buffer_!=NULL)
        free(buffer_);
}

void CharacterUtil::ReallocBuffer()
{
    bufferLen_*=2;
    buffer_=(char *)realloc(buffer_,bufferLen_);
    if(buffer_==NULL)
        throw std::bad_alloc();
}


