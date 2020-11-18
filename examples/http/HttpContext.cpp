//
// Created by wxl on 2020/11/12.
//

#include "HttpContext.h"
#include "net/Buffer.h"

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
                request_.setPath(start,question);
                request_.setQuery(question,space);
            }
            else
            {
                request_.setPath(start,space);
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
                    state_=kGotAll;
                    hasMore= false;
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
            //FIXME : add
        }
    }
    return ok;
}
