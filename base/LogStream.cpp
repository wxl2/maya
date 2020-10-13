//
// Created by wxl on 2020/10/12.
//

#include "LogStream.h"
#include <algorithm>

using namespace maya;
using namespace maya::detail;
namespace maya{
namespace detail{
    template class FixedBuffer<kSmallBuffer>;
    template class FixedBuffer<kLargeBuffer>;

    const char digits[] = "9876543210123456789";
    const char* zero = digits + 9;
    static_assert(sizeof(digits) == 20, "wrong number of digits");

    const char digitsHex[] = "0123456789ABCDEF";
    static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

    template <typename T>
    //将数字转换为十进制字符串
    size_t convert(char buf[], T value)
    {
        T i =value;
        char *p=buf;
        do {
            int lsd=static_cast<int>(value%10);
            *p++=zero[lsd];
        }while(i!=0);
        if(value<0)
        {
            *p++='-';
        }
        *p='\0';
        std::reverse(buf,p);
        return p - buf;//返回数字长度
    }

    //将数字转换为十进制字符串 //typedef unsigned int        uintptr_t;
    size_t convertHex(char buf[], uintptr_t value)
    {
        uintptr_t i = value;
        char* p = buf;

        do
        {
            int lsd = static_cast<int>(i % 16);
            i /= 16;
            *p++ = digitsHex[lsd];
        } while (i != 0);

        *p = '\0';
        std::reverse(buf, p);

        return p - buf;
    }
}
}

template <int SIZE>
const char * FixedBuffer<SIZE>::debugString() {
    *cur_='\0';
    return data_;
}

template <int SIZE>
void FixedBuffer<SIZE>::cookieEnd() {
}

template <int SIZE>
void FixedBuffer<SIZE>::cookieStart() {
}

template<typename T>
void LogStream::formatInteger(T v)
{
    if(buffer_.avail()>kMaxNumericSize)
    {
        size_t len=convert(buffer_.current(),v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.avail() >= kMaxNumericSize)
    {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf+2, v);
        buffer_.add(len+2);
    }
    return *this;
}

LogStream& LogStream::operator<<(double v)
{
    if (buffer_.avail() >= kMaxNumericSize)
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val)
{
    static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");

    length_ = snprintf(buf_, sizeof buf_, fmt, val);
    assert(static_cast<size_t>(length_) < sizeof buf_);
}

// 显示实例化

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);