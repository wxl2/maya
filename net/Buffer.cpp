//
// Created by wxl on 2020/11/2.
//

#include "Buffer.h"
#include "SocketsOps.h"

#include <sys/uio.h>
#include <errno.h>

using namespace maya;
using namespace maya::net;

const char Buffer::kCRLF[]="\r\n";

const size_t Buffer::kInitialSize;
const size_t Buffer::kCheapPrepend;

ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    ssize_t writeable=writeableBytes();
    vec[0].iov_base=begin()+writeIndex_;
    vec[0].iov_len=writeable;
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof(extrabuf);

    const int iovcnt=(writeable<sizeof(extrabuf))?2:1;
    const ssize_t n=sockets::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;
    }
    else if(implicit_cast<size_t>(n)<=writeable)
    {
        writeIndex_+=n;
    }
    else
    {
        writeIndex_=buffer_.size();
        append(extrabuf,n-writeable);
    }
    return n;
}
