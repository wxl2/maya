//
// Created by wxl on 2020/11/12.
//

#ifndef MAYA_HTTPCONTEXT_H
#define MAYA_HTTPCONTEXT_H

#include "base/copyable.h"
#include "HttpRequest.h"

namespace maya{
namespace net{

    class Buffer;

    class HttpContext:copyable
    {
    public:
        //期待处理的Http请求状态
        enum HttpRequestParseState
        {
            kExpectRequestLine,
            kExpectHeaders,
            kExpectBody,
            kGotAll,
        };

        HttpContext()
        :state_(kExpectRequestLine)
        {}

        bool parseRequest(Buffer* buf,Timestamp receiveTime);

        bool gotAll() const
        { return state_ == kGotAll; }

        void reset()
        {
            state_ = kExpectRequestLine;
            HttpRequest dummy;
            request_.swap(dummy);
        }

        const HttpRequest& request() const
        { return request_; }

        HttpRequest& request()
        { return request_; }
    private:
        //处理请求行
        bool processRequestLine(const char* begin,const char* end);

        HttpRequestParseState state_;
        HttpRequest request_;
    };
}//namespce net
}//namespace maya


#endif //MAYA_HTTPCONTEXT_H
