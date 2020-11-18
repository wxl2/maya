//
// Created by wxl on 2020/11/2.
//

#ifndef MAYA_BUFFER_H
#define MAYA_BUFFER_H
#include "base/copyable.h"
#include "Endian.h"
#include "base/Types.h"

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>

namespace maya{
namespace net{
    class Buffer:copyable
    {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialsize=kInitialSize)
        :buffer_(kCheapPrepend+initialsize),
        readIndex_(kCheapPrepend),
        writeIndex_(kCheapPrepend)
        {}

        void swap(Buffer& rhs)
        {
            buffer_.swap(rhs.buffer_);
            std::swap(readIndex_,rhs.readIndex_);
            std::swap(writeIndex_,rhs.writeIndex_);
        }

        //返回可读内容大小
        size_t readableBytes() const
        {
            return writeIndex_-readIndex_;
        }

        //返回可写内存大小
        size_t writeableBytes()
        {
            return buffer_.size()-writeIndex_;
        }

        //返回读起点标志
        size_t prependableBytes()
        {
            return readIndex_;
        }

        //返回读头的地址
        const char* peek() const
        {
            return begin()+readIndex_;
        }

        const char* beginWrite() const
        {
            return begin()+writeIndex_;
        }

        char * beginWrite()
        {
            return begin()+writeIndex_;
        }

        const char* findString(const char* targetStr) const
        {
            const char* found=std::search(peek(),beginWrite(),targetStr,targetStr+strlen(targetStr));
            return found==beginWrite()? nullptr:found;
        }

        const char* findCRLF() const
        {
            const char* crlf=std::search(peek(),beginWrite(),kCRLF,kCRLF+2);
            return crlf==beginWrite()? nullptr:crlf;
        }

        const char* findCRLF(const char* start) const
        {
            if(peek()>start)
                return nullptr;
            if(start>beginWrite())
                return nullptr;

            const char* crlf=std::search(peek(),beginWrite(),kCRLF,kCRLF+2);
            return crlf==beginWrite()? nullptr:crlf;
        }

        //查找\n,并返回其位置
        const char* findEOL() const
        {
            const void* eol=memchr(peek(),'\n',readableBytes());
            return static_cast<const char*>(eol);
        }

        const char* findEOL(const char* start) const
        {
            if(peek()>start)
                return nullptr;
            if(start>beginWrite())
                return nullptr;

            const void* eol=memchr(peek(),'\n',beginWrite()-start);
            return static_cast<const char*>(eol);
        }

        //从缓冲区中取出len长度的数据
        bool retrieve(size_t len)
        {
            if(len>readableBytes())//大于可读的长度
                return false;
            if(len<readableBytes())//小于
            {
                readIndex_+=len;
            }
            else//等于
            {
                retrieveAll();//全部取出
            }
            return true;
        }

        //从缓冲区中取出数据到end为止
        bool retrieveUntil(const char* end)
        {
            if(peek()>end)
                return false;
            if(end>beginWrite())
                return false;
            retrieve(end-peek());
            return true;
        }

        //取出64字节大小的数据
        void retrieve64()
        {
            retrieve(sizeof(int64_t));
        }

        void retrieve32()
        {
            retrieve(sizeof(int32_t));
        }

        void retrieve16()
        {
            retrieve(sizeof(int16_t));
        }

        void retrieve8()
        {
            retrieve(sizeof(int8_t));
        }

        void retrieveAll()
        {
            readIndex_=kCheapPrepend;
            writeIndex_=kCheapPrepend;
        }

        string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        string retrieveAsString(size_t len)
        {
            if(len>readableBytes())
                return "";

            string result(peek(),len);
            retrieve(len);
            return result;
        }

        string toStringPiece() const
        {
            return string(peek(),static_cast<int>(readableBytes()));
        }

        void append(const string& str)
        {
            append(str.c_str(),str.size());
        }

        void append(const char* data,size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data,data+len,beginWrite());
            hasWritten(len);
        }

        void append(const void* data,size_t len)
        {
            append(static_cast<const char*>(data),len);
        }

        bool ensureWritableBytes(size_t len)
        {
            if(writeableBytes()<len)
            {
                makeSpace(len);
            }
            return true;
        }

        bool hasWritten(size_t len)
        {
            if(len>writeableBytes())
                return false;
            writeIndex_+=len;
            return true;
        }

        bool unWrite(size_t len)
        {
            if(len>readableBytes())
                return false;
            writeIndex_-=len;
            return true;
        }

        void appendInt64(int64_t x)
        {
            int64_t be64=sockets::hostToNetwork64(x);
            append(&be64,sizeof(64));
        }


        void appendInt32(int32_t x)
        {
            int32_t be32=sockets::hostToNetwork32(x);
            append(&be32,sizeof(be32));
        }
        void appendInt16(int16_t x)
        {
            int16_t be16=sockets::hostToNetwoek16(x);
            append(&be16,sizeof(16));
        }
        void appendInt8(int8_t x)
        {
            append(&x,sizeof(x));
        }

        //read读出，会改变readIndex_
        int64_t readInt64()
        {
            int64_t result=peekInt64();
            retrieve64();
            return result;
        }

        int32_t readInt32()
        {
            int64_t result=peekInt32();
            retrieve32();
            return result;
        }

        int16_t readInt16()
        {
            int64_t result=peekInt16();
            retrieve16();
            return result;
        }

        int8_t readInt8()
        {
            int8_t result=peekInt8();
            retrieve8();
            return result;
        }

        //peek预查看,不改变readIndex_
        int64_t peekInt64()
        {
            if(readableBytes()<sizeof(int64_t))
                return -1;
            int64_t be64=0;
            ::memcpy(&be64,peek(),sizeof(be64));
            return sockets::networkToHost64(be64);
        }

        int32_t peekInt32()
        {
            if(readableBytes()<sizeof(int32_t))
                return -1;
            int32_t be32=0;
            ::memcpy(&be32,peek(),sizeof(be32));
            return sockets::networkToHost32(be32);
        }

        int16_t peekInt16()
        {
            if(readableBytes()<sizeof(int16_t))
                return -1;
            int16_t be16=0;
            ::memcpy(&be16,peek(),sizeof(be16));
            return sockets::networkToHost16(be16);
        }

        int8_t peekInt8()
        {
            if(readableBytes()<sizeof(int8_t))
                return -1;
            int8_t be8=*peek();
            return be8;
        }

        void prependInt64(int64_t x)
        {
            int64_t be64=sockets::hostToNetwork64(x);
            prepend(&be64,sizeof(be64));
        }

        void prependInt32(int32_t x)
        {
            int32_t be32 = sockets::hostToNetwork32(x);
            prepend(&be32, sizeof(be32));
        }

        void prependInt16(int16_t x)
        {
            int16_t be16 = sockets::hostToNetwoek16(x);
            prepend(&be16, sizeof(be16));
        }

        void prependInt8(int8_t x)
        {
            prepend(&x, sizeof(x));
        }

        bool prepend(const void * data,size_t len)
        {
            if(len>readableBytes())
                return false;
            readIndex_-=len;
            const char *d=static_cast<const char*>(data);
            std::copy(d,d+len,begin()+readIndex_);
            return true;
        }
        //缩小缓冲区,缩小后大小=readableBytes+reserve
        void shrink(size_t reserve)
        {
            Buffer other;
            other.ensureWritableBytes(readableBytes()+reserve);
            other.append(toStringPiece());
            swap(other);
        }

        size_t internalCapacity()
        {
            return buffer_.capacity();
        }

        ssize_t readFd(int fd,int *saveErrno);
    private:
        //返回缓冲区地址开始位置
        char *begin()
        {
            return &*buffer_.begin();
        }

        const  char* begin() const
        {
            return &*buffer_.begin();
        }

        void makeSpace(size_t len)
        {
            if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                //空间不足 FIXME: move readable data
                buffer_.resize(writeIndex_ + len);
            }
            else
            {
                //空间足够
                assert(kCheapPrepend < readIndex_);
                if (kCheapPrepend >= readIndex_)
                    return;

                size_t readable = readableBytes();
                std::copy(begin() + readIndex_,
                          begin() + writeIndex_,
                          begin() + kCheapPrepend);
                readIndex_ = kCheapPrepend;
                writeIndex_ = readIndex_ + readable;
                assert(readable == readableBytes());
            }

        }

    private:
        std::vector<char> buffer_;
        size_t readIndex_;
        size_t writeIndex_;

        static const char kCRLF[];
    };
}
}



#endif //MAYA_BUFFER_H
