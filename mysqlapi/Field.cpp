//
// Created by wxl on 2020/11/28.
//

#include "Field.h"

Field::Field():m_iType(DB_TYPE_UNKNOWN)
{
    m_bNULL= false;
}

Field::Field(Field &f)
{
    m_strFieldName=f.m_strFieldName;
    m_strValue=f.m_strValue;
    m_iType=f.m_iType;
}

Field::Field(const char *value, enum DataTypes type)
        :m_iType(type)
{
    m_strValue=value;
}

Field::~Field() {}

void Field::setValue(const char *value, size_t uLen)
{
    m_strValue.assign(value,uLen);
}