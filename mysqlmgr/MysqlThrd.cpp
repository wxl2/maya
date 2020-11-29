//
// Created by wxl on 2020/11/29.
//

#include "MysqlThrd.h"
#include <functional>
#include "base/Logging.h"

CMysqlThrd::CMysqlThrd(void)
{
    m_bStart= false;
    m_bTerminate= false;
    m_poConn=NULL;
}

CMysqlThrd::~CMysqlThrd(void)
{
}

void CMysqlThrd::Run()
{
    mainLoop();
    uninit();
    if(m_pThread!=NULL)
    {
        m_pThread->join();
    }
}

bool
CMysqlThrd::start(const std::string &host, const std::string &user, const std::string &pwd, const std::string &dbname)
{
    m_poConn=new CDatabaseMysql;
    if(m_poConn==NULL)
    {
        LOG_FATAL<<"CMysqlThrd::Start, Cannot open database";
        return false;
    }

    if(m_poConn->initialize(host,user,pwd,dbname)== false)
    {
        return false;
    }

    return init();
}

void CMysqlThrd::stop()
{
    if(m_bTerminate)
        return;
    m_bTerminate= true;
    m_pThread->join();
}

bool CMysqlThrd::init()
{
    if(m_bStart)
        return true;
    m_pThread.reset(new std::thread(std::bind(&CMysqlThrd::mainLoop, this)));

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(m_bStart== false)
        {
            cond_.wait(lock);
        }
    }
    return true;
}

void CMysqlThrd::uninit()
{
}

void CMysqlThrd::mainLoop()
{
    m_bStart= true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.notify_all();
    }

    IMysqlTask* poTask;

    while(!m_bTerminate)
    {
        if(NULL != (poTask = m_oTask.pop()))
        {
            poTask->execute(m_poConn);//取出要执行的任务进行执行,
            m_oReplyTask.push(poTask);// 完成之后将结果添加到返回队列中
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}





