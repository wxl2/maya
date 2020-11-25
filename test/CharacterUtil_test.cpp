//
// Created by wxl on 2020/11/25.
//
#include "utils/CharacterUtil.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main(int agrc,char* argv[])
{
/*    std::string str;
    str.append(1,0xc4);
    str.append(1,0xe3);
    str.append(1,0xba);
    str.append(1,0xc3);
    std::cout<<str<<std::endl;//GB2312编码的"你好"
    std::string dst;
    if(maya::detail::CharacterUtil::GB2312toUTF8(str,dst))
        std::cout<<dst;*/
    maya::detail::CharacterUtil tmp("gb2312","utf-8");
    int ifd=open("../../../flamingo/flamingoserver/base/AsyncLog.h",O_RDWR);
    if(ifd<0)
        perror("open");
    int ofd=open("./file.h",O_RDWR);
    if(ofd<0)
        perror("open");
    char *buf=new char [BUFSIZ];
    char *outbuf=new char [BUFSIZ];
    size_t len=BUFSIZ;
    size_t n=0;
    while((n =read(ifd,buf,12))>0)
    {
        tmp.GB2312toUTF8(buf,n,&outbuf,&len);
        printf("%s",outbuf);
        write(ofd,outbuf,len);
        memset(buf,0,BUFSIZ);
        len=BUFSIZ;
    }
    return 0;
}