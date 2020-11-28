//
// Created by wxl on 2020/11/27.
// C++二进制流协议类
//

#ifndef MAYA_PROTOCOLSTREAM_H
#define MAYA_PROTOCOLSTREAM_H

#include <stdio.h>
#include <sys/types.h>
#include <string>
#include <sstream>
#include <stdint.h>

namespace maya{
namespace detail{

    enum
    {
        TEXT_PACKLEN_LEN=4, //包头保存包体大小,头大小为4个字节
        TEXT_PACKAGE_MAXLEN=0xffff, //包体最大长度 保存为字节

        BINARY_PACKLEN_LEN = 2,
        BINARY_PACKAGE_MAXLEN = 0xffff,//保存为二进制流

        BINARY_PACKLEN_LEN_2 = 4,               //4字节头长度
        BINARY_PACKAGE_MAXLEN_2 = 0x10000000,   //包最大长度是256M,足够了

        CHECKSUM_LEN = 2,//校验位长度,2个字节
    };

    //计算校验和
    unsigned short checksum(const unsigned short* buffer,int size);

    //将一个4字节的整型数压缩成1~5个字节
    void write7BitEncoded(uint32_t value,std::string& buf);
    //将一个8字节的整型数压缩成1~5个字节
    void write7BitEncoded(uint64_t value,std::string& buf);
    //将一个1~5个字节的字符数组还原成4个字节整型数 , len为传出,返回还原了多少个字节
    void read7BitEncoded(const char* buf,uint32_t len,uint32_t& value);
    //将一个1~10个字节的字符数组还原成8个字节整型数 , len为传出,返回还原了多少个字节
    void read7BitEncoded(const char* buf,uint32_t len,uint64_t& value);

    class BinaryStreamWriter final
    {
    public:
        BinaryStreamWriter(std::string *data);
        ~BinaryStreamWriter()=default;

        virtual const char* GetData() const;//一方面这个类不能被继承,一方面设置虚函数,目的是运行是的多态,
                                            // 因为BinaryStreamReader和这个类有着同样的虚函数 BinaryStreamReader* p
                                            // 和BinaryStreamWriter *p 调用p->GetData所调用的函数是不同的,但这样一个函数
                                            // 的接口可以设置为void fun(BinaryStreamWriter* ptr),这个函数既可以传入
                                            // BinaryStreamWriter* p 也可以传入 BinaryStreamReader* p,但由于虚函数表的特性,
                                            // 两个类虚函数的定义顺序必须一样
        virtual size_t GetSize() const;

        bool WriteCString(const char* str,size_t len);
        bool WriteString(const std::string& str);
        bool WriteDouble(double value,bool isNULL= false);
        bool WriteInt64(uint64_t value,bool isNULL= false);
        bool WriteInt32(uint32_t i,bool isNULL= false);
        bool WriteShort(short s,bool isNULL= false);
        bool WriteChar(char c,bool isNULL= false);

        size_t GetCurrentPos() const
        {return m_data->length();}

        void Flush();
        void Clear();
    private:
        BinaryStreamWriter(const BinaryStreamWriter&)=delete;
        BinaryStreamWriter& operator=(const BinaryStreamWriter&)=delete;

        std::string* m_data;

    };

    class BinaryStreamReader final
    {
    public:
        BinaryStreamReader(const char* ptr,size_t len);
        ~BinaryStreamReader()=default;

        virtual const char* GetData() const;
        virtual size_t GetSize() const;

        bool IsEmpty() const;
        bool ReadString(std::string* str, size_t maxlen, size_t& outlen);
        bool ReadCString(char* str, size_t strlen, size_t& len);
        bool ReadCCString(const char** str, size_t maxlen, size_t& outlen);
        bool ReadInt32(int32_t& i);
        bool ReadInt64(int64_t& i);
        bool ReadShort(short& i);
        bool ReadChar(char& c);
        size_t ReadAll(char* szBuffer, size_t iLen) const;
        bool IsEnd() const;
        const char* GetCurrent() const { return cur_; }

    public:
        bool ReadLength(size_t& len);
        bool ReadLengthWithoutOffset(size_t& headlen, size_t& outlen);

    private:
        BinaryStreamReader(const BinaryStreamReader&) = delete;
        BinaryStreamReader& operator=(const BinaryStreamReader&) = delete;

    private:
        const char* const ptr_;
        const size_t len_;
        const char* cur_;
    };

}//namespace detail
}//namespace maya


#endif //MAYA_PROTOCOLSTREAM_H
