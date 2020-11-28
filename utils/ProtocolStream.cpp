//
// Created by wxl on 2020/11/27.
//

#include "ProtocolStream.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <cassert>
#include <algorithm>
#include <stdio.h>
#include <arpa/inet.h>

namespace maya{
namespace detail{
    unsigned short checksum(const unsigned short* buffer,int size)
    {
        //将数据按照16位全部想加求和而后取反
        unsigned int cksum=0;
        while(size>1)
        {
            cksum += *buffer;
            size -= sizeof(unsigned short);
        }
        if(size)//如果没能被16除尽,加上最后的
        {
            cksum += *(unsigned char*)buffer;
        }
        //将32位数转换成16
        while (cksum >> 16)
            cksum = (cksum >> 16) + (cksum & 0xffff);

        return (unsigned short)(~cksum);
    }

    void write7BitEncoded(uint32_t value,std::string& buf)
    {
        //其实就是消除value开头的0,做法将value分片,每个片大小为7bit
        do
        {
            unsigned char c= (unsigned char)(value&0x7f);//取出7bit 0x7f->0111 1111
            value >>= 7;
            if(value)//如果出现value不为0但是c为0,还原时判断结束会提前结束,所以将其与1000 0000相或使其不等于0
                c|=0x80;

            buf.append(1,c);
        }while(value);
    }

    void write7BitEncoded(uint64_t value, std::string& buf)
    {
        do
        {
            unsigned char c = (unsigned char)(value & 0x7F);
            value >>= 7;
            if (value)
                c |= 0x80;

            buf.append(1, c);
        } while (value);
    }

    void read7BitEncoded(const char* buf,uint32_t len,uint32_t& value)
    {
        char c;
        value = 0;
        int bitCount = 0;
        int index = 0;
        do
        {
            c = buf[index];
            uint32_t x = (c & 0x7F);
            x <<= bitCount;
            value += x;
            bitCount += 7;
            ++index;
        } while (c & 0x80);
    }

    void read7BitEncoded(const char* buf,uint32_t len,uint64_t& value)
    {
        char c;
        value=0;
        int bitCount=0;
        int index=0;
        do
        {
            c=buf[index];
            uint64_t x = (c&0x7f);
            x<<=bitCount;
            value += x;
            bitCount+=7;
            ++index;
        }while (c&0x80);
    }

    BinaryStreamWriter::BinaryStreamWriter(std::string *data)
    :m_data(data)
    {
        m_data->clear();
        char str[BINARY_PACKLEN_LEN_2+CHECKSUM_LEN];
        m_data->append(str,sizeof(str));//头部信息,一个保存包大小,一个保存校验和
    }

    const char *BinaryStreamWriter::GetData() const
    {
        return m_data->data();
    }

    size_t BinaryStreamWriter::GetSize() const
     {
        return m_data->length();
    }

    bool BinaryStreamWriter::WriteCString(const char *str, size_t len)
    {
        std::string  buf;
        write7BitEncoded(len,buf);
        m_data->append(buf);//如果buf为0x01 0x01那么此时m_data=0x01,0x01,0x00
        m_data->append(str,len);//如果str=0x02, m_data=0x01,0x01,0x02,0x00;那么如何划分m_data中的len头长度和str内容呢,
                                // 详解见BinaryStreamReader::ReadLengthWithoutOffset()
        return true;
    }

    bool BinaryStreamWriter::WriteString(const std::string &str)
     {
        return WriteCString(str.c_str(),str.length());
    }

    bool BinaryStreamWriter::WriteDouble(double value, bool isNULL)
    {
        char doubleStr[128];
        if(isNULL== false)
        {
            sprintf(doubleStr,"%f",value);
            WriteCString(doubleStr,strlen(doubleStr));
        }
        else
            WriteCString(doubleStr,0);//FIXME 此处为什么要写空串进去
        return true;
    }

    bool BinaryStreamWriter::WriteInt64(uint64_t value, bool isNULL)
    {
        char int64Str[128];
        if(isNULL== false)
        {
            sprintf(int64Str,"%ld",value);
            WriteCString(int64Str,strlen(int64Str));
        }
        else
            WriteCString(int64Str,0);
        return true;
    }

    bool BinaryStreamWriter::WriteInt32(uint32_t i, bool isNULL)
    {
        int32_t i2=999999999;
        if(isNULL== false)
            i2=htonl(i);
        m_data->append((char*)&i2,sizeof(i2));
        return true;
    }

    bool BinaryStreamWriter::WriteShort(short s, bool isNULL)
    {
        short i=0;
        if(isNULL== false)
            i=htons(s);
        m_data->append((char*)&i,sizeof(i));
        return true;
    }

    bool BinaryStreamWriter::WriteChar(char c, bool isNULL)
    {
        char c2=0;
        if(isNULL == false)
            c2=c;
        (*m_data)+=c2;
        return true;
    }

    void BinaryStreamWriter::Flush()
    {
        char* ptr = &(*m_data)[0];
        unsigned int ulen = htonl(m_data->length());
        memcpy(ptr, &ulen, sizeof(ulen));//将包体长度写如包头
    }

    void BinaryStreamWriter::Clear()
    {
        m_data->clear();
        char str[BINARY_PACKLEN_LEN_2+CHECKSUM_LEN];
        m_data->append(str,sizeof(str));
    }

    BinaryStreamReader::BinaryStreamReader(const char *ptr, size_t len)
    :ptr_(ptr),len_(len),cur_(ptr)
    {
        cur_+=BINARY_PACKLEN_LEN_2+CHECKSUM_LEN;
    }

    const char *BinaryStreamReader::GetData() const
    {
        return ptr_;
    }

    size_t BinaryStreamReader::GetSize() const
    {
        return len_;
    }

    bool BinaryStreamReader::IsEmpty() const
    {
        return len_<=BINARY_PACKLEN_LEN_2+CHECKSUM_LEN;
    }

    bool BinaryStreamReader::ReadLengthWithoutOffset(size_t &headlen, size_t &outlen)
    {
        //这个函数的作用是取出之前压缩的保存包头大小的头字段,之所以没有传入需要取出的大小即without offset
        // 是因为并不知道压缩后数的大小,并且headlen为传出参数,作用是传出头字段在data中所占用的长度,outlen传出的
        // 是头字段中保存的数的大小
        //headlen 包头长度 包头保存后面数据大小的字段
        headlen=0;
        const char* temp=cur_;
        char buf[5];
        for(size_t i=0;i<sizeof(buf);++i)
        {
            memcpy(buf+i,temp,sizeof(char));
            temp++;
            headlen++;
            if((buf[i]&0x80)==0x00)//这一步很关键,因为额write7BitEncoded()是按照7bit对数据进行压缩的,
                                   // 并且只有最后一位被压缩的数据是7bit,其他前面几位由于判断value是否为0时否被或上了0x80,
                                   // 所以前面几位数在次取出的都不为0,条件为假,不会跳出当前循环,唯有当取出最后一位时,因为最后一位
                                   // 最大为7bit所以无论如何取出的都是0,0x80->1000 0000 就会跳出当前循环,所以会在取出压缩数的
                                   // 最后一位数之后退出
                break; //FIXME
        }
        if(cur_+headlen>ptr_+len_)
            return false;
        unsigned int value;
        read7BitEncoded(buf,headlen,value);
        outlen=value;
        return true;
    }

    bool BinaryStreamReader::ReadLength(size_t &outlen)
    {
        size_t headlen;
        if(!ReadLengthWithoutOffset(headlen,outlen))
            return false;
        cur_+=headlen;
        return true;
    }

    bool BinaryStreamReader::ReadString(std::string *str, size_t maxlen, size_t &outlen)
    {
        size_t headlen;
        size_t fieldlen;

        if(!ReadLengthWithoutOffset(headlen,fieldlen))
            return false;
        if(maxlen!=0&&fieldlen>maxlen)
            return false;//传入的buf不足以容纳数据部分

        //偏移到数据位置
        cur_+=headlen;
        if(cur_+fieldlen>ptr_+len_)
        {
            outlen=0;
            return false;
        }
        str->assign(cur_,fieldlen);//取出数据
        outlen=fieldlen;
        cur_+=fieldlen;
        return true;
    }

    bool BinaryStreamReader::ReadCString(char *str, size_t strlen, size_t &outlen)
    {
        size_t headlen;
        size_t fieldlen;
        if(!ReadLengthWithoutOffset(headlen,fieldlen))
            return false;
        if(fieldlen>strlen)
            return false;

        cur_+=headlen;
        if(cur_+fieldlen>ptr_+len_)
        {
            outlen=0;
            return false;
        }
        memcpy(str,cur_,fieldlen);
        outlen=fieldlen;
        cur_+=fieldlen;
        return true;
    }

    bool BinaryStreamReader::ReadCCString(const char **str, size_t maxlen, size_t &outlen)
    {
        size_t headlen;
        size_t fieldlen;
        if(!ReadLengthWithoutOffset(headlen,fieldlen))
            return false;
        if(maxlen!=0&&fieldlen>maxlen)
            return false;
        cur_+=headlen;
        if(cur_+fieldlen>ptr_+len_)
        {
            outlen=0;
            return false;
        }

        *str=cur_;
        outlen=fieldlen;
        cur_+=fieldlen;
        return true;
    }

    bool BinaryStreamReader::ReadInt32(int32_t &i)
    {
        const int VALUE_SIZE=sizeof(int32_t);
        if(cur_+VALUE_SIZE>ptr_+len_)
            return false;
        memcpy(&i,cur_,VALUE_SIZE);
        i=ntohl(i);
        cur_+=VALUE_SIZE;
        return true;
    }

    bool BinaryStreamReader::ReadInt64(int64_t &i)
    {
        char int64Str[128];
        size_t length;
        if(!ReadCString(int64Str,128,length))
            return false;
        i=atoll(int64Str);
        return true;
    }

    bool BinaryStreamReader::ReadShort(short &i)
    {
        const int VALUE_SIZE=sizeof(short);
        if(cur_+VALUE_SIZE>ptr_+len_)
            return false;
        memcpy(&i,cur_,VALUE_SIZE);
        i=htons(i);
        cur_+=VALUE_SIZE;
        return true;
    }

    bool BinaryStreamReader::ReadChar(char &c)
    {
        const int VALUE_SIZE = sizeof(char);

        if (cur_ + VALUE_SIZE > ptr_ + len_) {
            return false;
        }

        memcpy(&c, cur_, VALUE_SIZE);
        cur_ += VALUE_SIZE;

        return true;
    }

    size_t BinaryStreamReader::ReadAll(char *szBuffer, size_t iLen) const
    {
        size_t isReadLen=std::min(iLen,len_);
        memcpy(szBuffer,cur_,isReadLen);
        return isReadLen;
    }

    bool BinaryStreamReader::IsEnd() const
    {
        assert(cur_<=ptr_+len_);
        return cur_==ptr_+len_;
    }


}//namespace detail
}//namespace maya

