//
// Created by wxl on 2020/11/28.
//

#ifndef MAYA_QUERYRESULT_H
#define MAYA_QUERYRESULT_H

#include "Field.h"
#include <mysql.h>
#include <stdint.h>
#include <vector>
#include <map>
/**
 * 对MySQL数据库进行操作
 * 执行SQL
 */

class QueryResult
{
public:
    typedef std::map<uint32_t, std::string> FieldNames;
    QueryResult(MYSQL_RES* result,uint64_t rowCount,uint32_t fieldCount);
    virtual ~QueryResult();

    virtual bool nextRow();

    uint32_t getFieldIdx(const std::string& name) const;
    Field* fetch() const{return m_CurrentRow;}

    const Field & operator[](int index)const
    { return m_CurrentRow[index]; }

    const Field & operator[](const std::string& name) const
    { return  m_CurrentRow[getFieldIdx(name)];}

    uint32_t getFieldCount()const{ return  m_FieldCount; }
    uint64_t getRowCount() const{ return m_RowCount; }
    const FieldNames & getFieldNames() const{return m_FieldNames;}

    const std::vector<std::string>& getNames(){return m_vtFieldNames;}

private:
    enum Field::DataTypes convertNativeType(enum_field_types mysqlType) const;
public:
    void endQuery();
protected:
    Field*                      m_CurrentRow; //保存当前行的数据
    uint32_t                    m_FieldCount;//二维表列的长度
    uint64_t                    m_RowCount;//二维表行的长度
    FieldNames                  m_FieldNames;//保存表头
    std::vector<std::string>    m_vtFieldNames;//保存表头
    MYSQL_RES*                  m_Result;
    /**
     * a eg:
     * +----+-----------+
     * | id | name      |
     * +----+-----------+
     * |  1 | 张三      |
     * |  2 | 鏉庡洓    |
     * |  3 | lll       |
     * +----+-----------+
     * 在执行QueryResult::QueryResult()时m_CurrentRow的
     * 表头字段m_strFieldName和m_iType会被设置,同时
     * m_FieldCount被设置为2,m_RowCount被设置
     * 为3,执行一个CDatabaseMysql::query()后m_CurrentRow会被指向第1行
     * 同时会设置m_CurrentRow的m_strValue字段值
     */
};


#endif //MAYA_QUERYRESULT_H
