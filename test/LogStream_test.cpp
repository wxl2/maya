//
// Created by wxl on 2020/10/12.
//

#include "base/LogStream.h"
#include <iostream>

using namespace maya;
int main()
{
    char buf[64]="test buffer";
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
    Buffer buffer_;
    buffer_.append(buf,strlen(buf));
    while(1);
    return 0;
}

