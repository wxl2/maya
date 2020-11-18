//
// Created by wxl on 2020/11/12.
//

#ifndef MAYA_HTTPREQUEST_H
#define MAYA_HTTPREQUEST_H

#include "base/nocopyable.h"
#include "base/Types.h"
#include "base/Timestamp.h"

#include <map>
#include <assert.h>
#include <stdio.h>

/*HTTP 请求方法
根据 HTTP 标准，HTTP 请求可以使用多种请求方法。

HTTP1.0 定义了三种请求方法： GET, POST 和 HEAD方法。

HTTP1.1 新增了六种请求方法：OPTIONS、PUT、PATCH、DELETE、TRACE 和 CONNECT 方法。

序号	方法	描述
1	GET	请求指定的页面信息，并返回实体主体。
2	HEAD	类似于 GET 请求，只不过返回的响应中没有具体的内容，用于获取报头
3	POST	向指定资源提交数据进行处理请求（例如提交表单或者上传文件）。数据被包含在请求体中。POST 请求可能会导致新的资源的建立和/或已有资源的修改。
4	PUT	从客户端向服务器传送的数据取代指定的文档的内容。
5	DELETE	请求服务器删除指定的页面。
6	CONNECT	HTTP/1.1 协议中预留给能够将连接改为管道方式的代理服务器。
7	OPTIONS	允许客户端查看服务器的性能。
8	TRACE	回显服务器收到的请求，主要用于测试或诊断。
9	PATCH	是对 PUT 方法的补充，用来对已知资源进行局部更新 。*/

//GET /index.html HTTP/1.1\r\n  ///请求行
//Host: 127.0.0.1:8080\r\n
//Connection: keep-alive\r\n
//Upgrade-Insecure-Requests: 1\r\n
//User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36\r\n
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n
//Sec-Fetch-Site: none\r\n
//Sec-Fetch-Mode: navigate\r\n
//Sec-Fetch-User: ?1\r\n
//Sec-Fetch-Dest: document\r\n
//Accept-Encoding: gzip, deflate, br\r\n
//Accept-Language: zh-CN,zh;q=0.9\r\n   ///请求头
//\r\n  ///空行
//username=aa&password=1234\r\n   ///请求数据


namespace maya{
namespace net{
    //Http请求
    class HttpRequest:copyable
    {
    public:
        enum Method//Http Method
        {
            kInvalid,kGet,kPost,kHead,kPut,kDelete
        };

        enum Version//Http version
        {
            kUnknown,kHttp10,kHttp11
        };
        HttpRequest()
        :method_(kInvalid),
        version_(kUnknown)
        {}

        void setVersion(Version version)
        {version_=version;}
        Version getVersion() const
        { return version_; }

        bool setMethod(const char* start,const char* end)//传入一个请求方法的字符串
        {
            assert(method_==kInvalid);
            string m(start,end);
            if(m=="GET")
            {
                method_=kGet;
            }
            else if (m == "POST")
            {
                method_ = kPost;
            }
            else if (m == "HEAD")
            {
                method_ = kHead;
            }
            else if (m == "PUT")
            {
                method_ = kPut;
            }
            else if (m == "DELETE")
            {
                method_ = kDelete;
            }
            else
            {
                method_ = kInvalid;
            }
            return method_ != kInvalid;
        }

        Method method() const
        {return method_;}

        const char* methodString() const
        {
            const char* result = "UNKNOWN";
            switch(method_)
            {
                case kGet:
                    result = "GET";
                    break;
                case kPost:
                    result = "POST";
                    break;
                case kHead:
                    result = "HEAD";
                    break;
                case kPut:
                    result = "PUT";
                    break;
                case kDelete:
                    result = "DELETE";
                    break;
                default:
                    break;
            }
            return result;
        }

        //assign方法可以理解为先将原字符串清空，然后赋予新的值作替换
        void setPath(const char* start, const char* end)
        {
            path_.assign(start, end);
        }

        const string& path() const
        { return path_; }

        void setQuery(const char* start, const char* end)
        {
            query_.assign(start, end);
        }

        const string& query() const
        { return query_; }

        void setReceiveTime(Timestamp t)
        { receiveTime_ = t; }

        Timestamp receiveTime() const
        { return receiveTime_; }

        //传入一行数据,colon为请求头的:分割
        void addHeader(const char* start,const char* colon,const char*end)
        {
            string field(start,colon);
            ++colon;
            if(colon<end&&isspace(*colon))
            {
                ++colon;//去除首部空格
            }
            string value(colon,end);
            while (!value.empty()&&isspace(value[value.size()-1]))//去除尾部空格
            {
                value.resize(value.size()-1);
            }
            headers_[field]=value;
        }

        string getHeader(const string& field) const
        {
            string result;
            std::map<string, string>::const_iterator it = headers_.find(field);
            if (it != headers_.end())
            {
                result = it->second;
            }
            return result;
        }

        const std::map<string, string>& headers() const
        { return headers_; }

        void swap(HttpRequest& that)
        {
            std::swap(method_, that.method_);
            std::swap(version_, that.version_);
            path_.swap(that.path_);
            query_.swap(that.query_);
            receiveTime_.swap(that.receiveTime_);
            headers_.swap(that.headers_);
        }

    private:
        Method method_;
        Version version_;
        string path_;//http请求行的url
        string query_;//http请求行?后的参数
        Timestamp receiveTime_;//接收到请求的时间
        std::map<string ,string > headers_;//Http请求头
    };
}//namespace net
}//namespace maya

#endif //MAYA_HTTPREQUEST_H
