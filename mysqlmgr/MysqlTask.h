//
// Created by wxl on 2020/11/29.
//

#ifndef MAYA_MYSQLTASK_H
#define MAYA_MYSQLTASK_H
enum EMysqlError
{
    EME_ERROR=-1,
    EME_OK,
    EME_NO_EXIST,
    EME_EXIST,
};

class CDatabaseMysql;

class IMysqlTask
{
public:
    IMysqlTask(){}
    virtual ~IMysqlTask(){}

public:
    virtual void execute(CDatabaseMysql* poConn)=0;
    virtual void reply()=0;
    virtual void release(){delete this;}
};
#endif //MAYA_MYSQLTASK_H
