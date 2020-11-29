//
// Created by wxl on 2020/11/28.
//

#include "DatabaseMysql.h"
#include "base/Logging.h"
#include <fstream>
#include <stdarg.h>
#include <string.h>

CDatabaseMysql::CDatabaseMysql()
{
    m_Mysql=NULL;
    m_bInit= false;
}

CDatabaseMysql::~CDatabaseMysql()
{
    if(m_Mysql!=NULL)
    {
        if(m_bInit)
            mysql_close(m_Mysql);
    }
}

bool CDatabaseMysql::initialize(const std::string &host, const std::string &user, const std::string &pwd,
                                const std::string &dbname)
{
    if(m_bInit)
    {
        mysql_close(m_Mysql);
    }
    m_Mysql=mysql_init(m_Mysql);
    m_Mysql=mysql_real_connect(m_Mysql,host.c_str(),user.c_str(),pwd.c_str(),dbname.c_str(),0,NULL,0);

    m_DBInfo.strHost=host;
    m_DBInfo.strUser=user;
    m_DBInfo.strPwd=pwd;
    m_DBInfo.strDBName=dbname;

    if(m_Mysql)
    {
       mysql_query(m_Mysql,"set names utf8");
       m_bInit= true;
        return true;
    }
    else
    {
        mysql_close(m_Mysql);
        return false;
    }


    return false;
}

QueryResult *CDatabaseMysql::query(const char *sql)
{
    if(!m_Mysql)
    {
        if(initialize(m_DBInfo.strHost,m_DBInfo.strUser,m_DBInfo.strPwd,m_DBInfo.strDBName)
        == false)
            return NULL;
    }
    if(!m_Mysql)
        return NULL;

    MYSQL_RES* result=NULL;
    uint64_t rowCount=0;
    uint32_t fieldCount=0;

    {
        int iTempRet = mysql_real_query(m_Mysql, sql, strlen(sql));
        if (iTempRet) {
            unsigned int uErrno = mysql_errno(m_Mysql);
            if (CR_SERVER_GONE_ERROR == uErrno) {
                if (false == initialize(m_DBInfo.strHost, m_DBInfo.strUser,
                                        m_DBInfo.strPwd, m_DBInfo.strDBName)) {
                    return NULL;
                }
                iTempRet = mysql_real_query(m_Mysql, sql, strlen(sql));
                if (iTempRet) {
                    LOG_ERROR << "SQL: " << sql;
                    LOG_ERROR << "query ERROR: " << mysql_error(m_Mysql);
                }
            } else {
                LOG_ERROR << "SQL: " << sql;
                LOG_ERROR << "query ERROR: " << mysql_error(m_Mysql);
                return NULL;
            }
        }
    }
    result = mysql_store_result(m_Mysql);

    rowCount = mysql_affected_rows(m_Mysql);
    fieldCount = mysql_field_count(m_Mysql);

    if (!result)
        return NULL;

    QueryResult* queryResult = new QueryResult(result, rowCount, fieldCount);

    queryResult->nextRow();

    return queryResult;
}

QueryResult *CDatabaseMysql::pquery(const char *format, ...)
{
    if (!format) return NULL;

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if (res == -1)
    {
        LOG_ERROR << "SQL Query truncated (and not execute) for format: " << format;
        return NULL;
    }

    return query(szQuery);
}

bool CDatabaseMysql::execute(const char *sql)
{
    if (!m_Mysql)
        return false;

    {
        int iTempRet = mysql_query(m_Mysql, sql);
        if (iTempRet)
        {
            unsigned int uErrno = mysql_errno(m_Mysql);
            if (CR_SERVER_GONE_ERROR == uErrno)
            {
                if (false == initialize(m_DBInfo.strHost, m_DBInfo.strUser, m_DBInfo.strPwd, m_DBInfo.strDBName))
                {
                    return false;
                }
                iTempRet = mysql_real_query(m_Mysql, sql, strlen(sql));
                if (iTempRet)
                {
                    LOG_ERROR<<"sql error: "<<mysql_error(m_Mysql)<< sql;
                }
            }
            else
            {
                LOG_ERROR<<"sql error: "<<mysql_error(m_Mysql)<< sql;
            }
            return false;
        }
    }
    return true;
}

bool CDatabaseMysql::execute(const char *sql, uint32_t &uAffectedCount, int &nErrno)
{
    if (!m_Mysql)
        return false;

    {
        int iTempRet = mysql_query(m_Mysql, sql);
        if (iTempRet)
        {
            unsigned int uErrno = mysql_errno(m_Mysql);
            if (CR_SERVER_GONE_ERROR == uErrno)
            {
                if (false == initialize(m_DBInfo.strHost, m_DBInfo.strUser,
                                        m_DBInfo.strPwd, m_DBInfo.strDBName))
                {
                    return false;
                }
                iTempRet = mysql_query(m_Mysql, sql);
                nErrno = iTempRet;
                if (iTempRet)
                {
                    LOG_ERROR << "SQL: " << sql;
                    LOG_ERROR << "query ERROR: " << mysql_error(m_Mysql);
                }
            }
            else
            {
                LOG_ERROR << "SQL: " << sql;
                LOG_ERROR << "query ERROR: " << mysql_error(m_Mysql);
            }
            return false;
        }
        uAffectedCount = static_cast<uint32_t>(mysql_affected_rows(m_Mysql));
    }
    return true;
}

bool CDatabaseMysql::pexecute(const char *format, ...)
{
    if (!format)
        return false;

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if (res == -1)
    {
        LOG_ERROR << "SQL Query truncated (and not execute) for format: " << format;
        return false;
    }

    if (!m_Mysql)
        return false;

    {
        int iTempRet = mysql_query(m_Mysql, szQuery);
        if (iTempRet)
        {
            unsigned int uErrno = mysql_errno(m_Mysql);
            if (CR_SERVER_GONE_ERROR == uErrno)
            {
                if (false == initialize(m_DBInfo.strHost, m_DBInfo.strUser,
                                        m_DBInfo.strPwd, m_DBInfo.strDBName))
                {
                    return false;
                }
                iTempRet = mysql_query(m_Mysql, szQuery);
                if (iTempRet)
                {
                    LOG_ERROR << "SQL: " << szQuery;
                    LOG_ERROR << "query ERROR: " << mysql_error(m_Mysql);
                }
            }
            else
            {
                LOG_ERROR << "SQL: " << szQuery;
                LOG_ERROR << "query ERROR: " << mysql_error(m_Mysql);
            }
            return false;
        }
    }
    return true;
}

uint32_t CDatabaseMysql::getInsertID()
{
    return (uint32_t)mysql_insert_id(m_Mysql);
}

void CDatabaseMysql::clearStoredResults()
{
    if (!m_Mysql)
    {
        return;
    }

    MYSQL_RES* result = NULL;
    while (!mysql_next_result(m_Mysql))
    {
        if ((result = mysql_store_result(m_Mysql)) != NULL)
        {
            mysql_free_result(result);
        }
    }
}

int32_t CDatabaseMysql::escapeString(char *szDst, const char *szSrc, uint32_t uSize)
{
    if (m_Mysql == NULL)
    {
        return 0;
    }
    if (szDst == NULL || szSrc == NULL)
    {
        return 0;
    }

    return mysql_real_escape_string(m_Mysql, szDst, szSrc, uSize);
}

