//
// Created by wxl on 2020/11/28.
//

#ifndef MAYA_FIELD_H
#define MAYA_FIELD_H

/**
 * Filed是一个数据 结构,仅保存MySQL二维表中具体某行某列的值,
 * 并且还保存了其所在的列表头,以及数据类型(和表头数据类型相同)
 * 当m_bNULL为false时保存的值才有效
 */

#include <algorithm>
#include <string>


inline void toLowerString(std::string& str)
{
    for(size_t i=0;i<str.size();++i)
    {
        if(str[i]>='A'&&str[i]<='Z')
            str[i]+=32;
    }
}

class Field
{
public:
    enum DataTypes
    {
        DB_TYPE_UNKNOWN = 0x00,
        DB_TYPE_STRING  = 0x01,
        DB_TYPE_INTEGER = 0x02,
        DB_TYPE_FLOAT   = 0x03,
        DB_TYPE_BOOL    = 0x04
    };

    Field();
    Field(Field& f);
    Field(const char* value,enum DataTypes type);

    ~Field();

    enum DataTypes getType() const{return m_iType;}
    const std::string getString() const {return m_strValue;}

    std::string getCppString() const
    {
        return m_strValue;
    }

    float getFloat() const{return static_cast<float>(atof(m_strValue.c_str()));}
    bool getBool() const{return atoi(m_strValue.c_str())>0;}
    int32_t getInt32() const{return static_cast<int32_t>(atol(m_strValue.c_str()));}
    uint32_t getUInt32() const{return static_cast<uint32_t>(atol(m_strValue.c_str()));}
    uint8_t getUInt8() const{return static_cast<uint8_t>(atol(m_strValue.c_str()));}
    uint16_t getUInt16() const{return static_cast<uint16_t>(atol(m_strValue.c_str()));}
    int16_t getInt16() const{return static_cast<int16_t>(atol(m_strValue.c_str()));}
    uint64_t getUInt64() const
    {
        uint64_t value=0;
        value=atoll(m_strValue.c_str());
        return value;
    }

    void setType(enum DataTypes type){m_iType=type;}

    void setValue(const char* value,size_t uLen);
    void setName(const std::string& strName)
    {
        m_strFieldName=strName;
        toLowerString(m_strFieldName);
    }

    const std::string getName() const{return m_strFieldName;}

    bool isNULL() const{return m_bNULL;}

    template<typename T>
    void convertValue(T& value);

public:
    bool m_bNULL;//表示mysql的该字段是否为空

private:
    std::string m_strValue;//字段值
    std::string m_strFieldName;//表头,字段名
    enum DataTypes m_iType;//字段的数据类型

};


#endif //MAYA_FIELD_H
