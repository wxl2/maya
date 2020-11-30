//
// Created by wxl on 2020/11/30.
//

#ifndef MAYA_TCPSESSION_H
#define MAYA_TCPSESSION_H

#include "base/nocopyable.h"
#include "net/TcpConnection.h"
#include <memory>

namespace maya{
namespace net{

    class TcpSession: public nocopyable
    {
    public:
        TcpSession(const std::weak_ptr<TcpConnection>& tmpConn);
        ~TcpSession();

        std::shared_ptr<TcpConnection> getConnectionPtr()
        {
            if(tmpConn_.expired())
                return NULL;
            return tmpConn_.lock();
        }
        /*!
         *
         * @param cmd 标识文件为上传还是下载
         * @param seq 数据包序列号
         * @param errorcode 错误码
         * @param filemd5 文件的md5值
         * @param offset 文件传输的偏远量
         * @param filesize 文件大小
         * @param filedata 文件内容
         */
        void send(int32_t cmd,int32_t seq,int32_t errorcode,const std::string& filemd5,int64_t offset,int64_t filesize, const std::string& filedata);

    private:
        /*!
         * 支持大文件传输,文件长度使用int64_t保存
         * @param body 传输内容头指针
         * @param bodylength 传输内容大小
         */
        void sendPackage(const char* body,int64_t bodylength);
    protected:
        std::weak_ptr<TcpConnection> tmpConn_;
    };
}//namespace net
}//namespace maya


#endif //MAYA_TCPSESSION_H
