//
// Created by wxl on 2020/11/29.
//

#ifndef MAYA_MYSQLTHRD_H
#define MAYA_MYSQLTHRD_H

#include <thread>
#include <condition_variable>

#include "mysqlapi/DatabaseMysql.h"
#include "MysqlTask.h"
#include "TaskList.h"

class CMysqlThrd
{
public:
    CMysqlThrd(void);
    ~CMysqlThrd(void);

    void Run();

    bool start(const std::string& host, const std::string& user, const std::string& pwd, const std::string& dbname);
    void stop();
    bool addTask(IMysqlTask* poTask)
    {
        return m_oTask.push(poTask);
    }

    IMysqlTask* getReplyTask(void)
    {
        return m_oReplyTask.pop();
    }
protected:
    bool init();
    void mainLoop();
    void uninit();
private:
    bool                            m_bTerminate;
    std::unique_ptr<std::thread>    m_pThread;
    bool                            m_bStart;
    CDatabaseMysql*                 m_poConn;
    CTaskList                       m_oTask;
    CTaskList                       m_oReplyTask;

    std::mutex                      mutex_;
    std::condition_variable         cond_;
};


#endif //MAYA_MYSQLTHRD_H
