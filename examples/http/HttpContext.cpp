//
// Created by wxl on 2020/11/12.
//

#include "HttpContext.h"
#include "net/Buffer.h"
#include "utils/URLEncodeUtil.h"
using namespace maya;
using namespace maya::net;


//GET /index.html?id=1 HTTP/1.1\r\n  ///请求行
bool HttpContext::processRequestLine(const char *begin, const char *end)
{
    bool succeed= false;
    const char* start=begin;
    const char* space =std::find(begin,end,' ');
    if(space!=end&&request_.setMethod(begin,space))
    {
        start=space+1;
        space=std::find(start,end,' ');
        if(space!=end)
        {
            const char* question=std::find(start,space,'?');
            if(space!=end)
            {
                std::string src(start,question);
                std::string dst;
                detail::URLEncodeUtil::decode(src,dst);
                request_.setPath(dst.c_str(),dst.c_str()+dst.size());
                request_.setQuery(question,space);
            }
            else
            {
                std::string src(start,question);
                std::string dst;
                detail::URLEncodeUtil::encode(src,dst);
                request_.setPath(dst.c_str(),dst.c_str()+dst.size());
            }

            start=space+1;
            succeed=end-start==8&&std::equal(start,end-1,"HTTP/1.");
            if(succeed)
            {
                if(*(end-1)=='1')
                {
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else if(*(end-1)=='0')
                {
                    request_.setVersion(HttpRequest::kHttp10);
                }
                else
                {
                    succeed= false;
                }
            }
        }
    }
    return succeed;
}

bool HttpContext::parseRequest(Buffer *buf, Timestamp receiveTime)
{
    bool ok= true;
    bool hasMore= true;
    while (hasMore)
    {
        if(state_==kExpectRequestLine)
        {
            const char* clrf=buf->findCRLF();
            if(clrf)
            {
                ok =processRequestLine(buf->peek(),clrf);
                if(ok)
                {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(clrf+2);
                    state_=kExpectHeaders;
                }
                else
                {
                    hasMore= false;
                }
            }
            else
            {
                hasMore= false;
            }
        }
        else if(state_==kExpectHeaders)
        {
            const char* clrf=buf->findCRLF();
            if(clrf)
            {
                //Host: 127.0.0.1:8080\r\n
                const char* colon=std::find(buf->peek(),clrf,':');
                if(colon!=clrf)
                {
                    request_.addHeader(buf->peek(),colon,clrf);
                }
                else
                {
                    state_=kExpectBody;
                    if(request_.method()==HttpRequest::kGet)
                    {
                        state_ = kGotAll;
                        hasMore = false;
                    }
                }
                buf->retrieveUntil(clrf+2);
            }
            else
            {
                hasMore= false;
            }
        }
        else if(state_==kExpectBody)
        {
            //FIXME : add 处理请求体部分
            request_.setBody(string(buf->peek(),buf->readableBytes()));
            buf->retrieveAll();
            state_=kGotAll;
            hasMore= false;
        }
    }
    return ok;
}
