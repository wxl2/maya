//
// Created by wxl on 2020/11/28.
//

#include "utils/ProtocolStream.h"
#include <iostream>
#include <stdio.h>
using namespace maya;
using namespace maya::detail;
int main()
{
//    uint64_t in2=85554;
//    uint64_t out2;
//    std::string buf,data;
//    buf.clear();
//    write7BitEncoded(in2,buf);
//    data.append(buf);
//    char str[2]={'s','s'};
//    data.append(str,2);
//    for(int i=0;i<buf.size();++i)
//        printf("%d ",buf[i]);
//    printf("\n");
//    uint32_t len=0;
//    read7BitEncoded(buf.c_str(),len,out2);
//    std::cout<<"len: "<<len<<" value: "<<out2<<std::endl;
    std::string *str=new std::string;
    BinaryStreamWriter writer(str);
//    int64_t i=1558;
//    writer.WriteInt64(i);
//    int64_t out;
//    reader.ReadInt64(out);
//    std::cout<<out<<std::endl;
    std::string s="dadadasd";
    writer.WriteString(s);
    BinaryStreamReader reader(writer.GetData(),writer.GetSize());
    std::string s2;
    size_t len;
    reader.ReadString(&s2,56,len);
    std::cout<<"len: "<<len<<" data: "<<s2<<" size: "<<s2.size();
    return 0;
}