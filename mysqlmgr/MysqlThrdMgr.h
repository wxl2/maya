//
// Created by wxl on 2020/11/29.
//

#ifndef MAYA_MYSQLTHRDMGR_H
#define MAYA_MYSQLTHRDMGR_H

#include "MysqlTask.h"
#include "MysqlThrd.h"

class MysqlThrdMgr
{
public:
    MysqlThrdMgr();
    ~MysqlThrdMgr();

public:
    bool init(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname);
    bool addTask(uint32_t dwHashID, IMysqlTask* poTask);
    bool addTask(IMysqlTask* poTask)
    {
        return m_aoMysqlThreads[m_dwThreadsCount].addTask(poTask);
    }

    inline uint32_t getTableHashID(uint32_t dwHashID) const
    {
        return dwHashID % m_dwThreadsCount;
    }

    bool processReplyTask(int32_t nCount);
    static uint32_t getThreadsCount()
    {
        return m_dwThreadsCount;
    }

protected:
    static const uint32_t m_dwThreadsCount=9;
    CMysqlThrd m_aoMysqlThreads[m_dwThreadsCount+1];
};


#endif //MAYA_MYSQLTHRDMGR_H
