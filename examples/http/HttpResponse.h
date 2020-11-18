//
// Created by wxl on 2020/11/12.
//

#ifndef MAYA_HTTPRESPONSE_H
#define MAYA_HTTPRESPONSE_H

#include "base/copyable.h"
#include "base/Types.h"

#include <map>

namespace maya{
namespace net{
    class Buffer;
    //Http响应
    class HttpResponse:copyable
    {
    public:
        //Http响应码
        enum HttpStatusCode
        {
            kUnknown,
            k200Ok=200,
            k301MovedPermanently = 301,
            k400BadRequest = 400,
            k404NotFound = 404,
            k500ServerError=500
        };

        explicit HttpResponse(bool close)
        :closeConnection_(close),
        statusCode_(kUnknown)
        {}

        void setStatusCode(HttpStatusCode code)
        { statusCode_ = code; }

        void setStatusMessage(const string& message)
        { statusMessage_ = message; }

        void setCloseConnection(bool on)
        { closeConnection_ = on; }

        bool closeConnection() const
        { return closeConnection_; }

        void setContentType(const string& contentType)
        { addHeader("Content-Type", contentType); }

        void setContentLanguage(const string& ContentLanguage)
        {addHeader("Content-Language",ContentLanguage);}
        void addHeader(const string& key, const string& value)
        { headers_[key] = value; }

        void setBody(const string& body)
        { body_ = body; }

        void appendToBuffer(Buffer* output) const;

    private:
        std::map<string ,string > headers_;
        HttpStatusCode statusCode_;
        string statusMessage_;
        bool closeConnection_;
        string body_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_HTTPRESPONSE_H
