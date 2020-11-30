//
// Created by wxl on 2020/11/30.
//

#include "TcpSession.h"
#include "FileMsg.h"
#include "utils/ProtocolStream.h"
#include "base/Logging.h"

maya::net::TcpSession::TcpSession(const std::weak_ptr<TcpConnection> &tmpConn)
:tmpConn_(tmpConn)
{
}

maya::net::TcpSession::~TcpSession()
{
}

void
maya::net::TcpSession::send(int32_t cmd, int32_t seq, int32_t errorcode, const std::string &filemd5, int64_t offset,
                            int64_t filesize, const std::string &filedata)
{
    std::string outbuf;
    maya::detail::BinaryStreamWriter writerStream(&outbuf);
    writerStream.WriteInt32(cmd);
    writerStream.WriteInt32(seq);
    writerStream.WriteInt32(errorcode);
    writerStream.WriteString(filemd5);
    writerStream.WriteInt64(offset);
    writerStream.WriteInt64(filesize);
    writerStream.WriteString(filedata);

    writerStream.Flush();
    sendPackage(outbuf.c_str(),outbuf.length());
}

void maya::net::TcpSession::sendPackage(const char *body, int64_t bodylength)
{
    string strPackageData;
    file_msg_header header = { (int64_t)bodylength };
    strPackageData.append((const char*)&header, sizeof(header));
    strPackageData.append(body, bodylength);

    if (tmpConn_.expired())
    {
        LOG_ERROR<<"Tcp connection is destroyed , but why TcpSession is still alive ?";
        return;
    }

    std::shared_ptr<TcpConnection> conn = tmpConn_.lock();
    if (conn)
    {
        LOG_INFO<<"Send data, package length: "<<strPackageData.length()<<" body length: %d"<<bodylength;
        conn->send(strPackageData.c_str(), strPackageData.length());
    }
}
