//
// Created by wxl on 2020/11/29.
//

#include "MysqlThrdMgr.h"

MysqlThrdMgr::MysqlThrdMgr()
{
}

MysqlThrdMgr::~MysqlThrdMgr()
{
}

bool MysqlThrdMgr::init(const std::string &host, const std::string &user, const std::string &pwd,
                        const std::string &dbname)
{
    for(uint32_t i=0;i<m_dwThreadsCount+1;++i)
    {
        if(m_aoMysqlThreads[i].start(host,user,pwd,dbname)== false)
            return false;
    }

    return true;
}

bool MysqlThrdMgr::addTask(uint32_t dwHashID, IMysqlTask *poTask)
{
    uint32_t btIndex=getTableHashID(dwHashID);
    if(btIndex>m_dwThreadsCount)
        return false;

    return m_aoMysqlThreads[btIndex].addTask(poTask);
}

bool MysqlThrdMgr::processReplyTask(int32_t nCount)
{
    /**
     * 从每个线程中取出nCount个mysql响应,如果nCount==-1,表示取出全部
     */
    bool bResult = false;

    for (uint32_t i = 0; i < m_dwThreadsCount + 1; ++i)
    {
        IMysqlTask* poTask = m_aoMysqlThreads[i].getReplyTask();
        int32_t dwProcessedNbr = 0;

        while (((nCount == -1) || (dwProcessedNbr < nCount)) && (NULL != poTask))
        {
            poTask->reply();
            poTask->release();
            poTask = m_aoMysqlThreads[i].getReplyTask();
            ++dwProcessedNbr;
        }

        if (dwProcessedNbr == nCount)
        {
            bResult = true;
        }
    }

    return bResult;
}
