//
// Created by wxl on 2020/11/29.
//
#include "mysqlapi/DatabaseMysql.h"
#include <iostream>

int main()
{
    CDatabaseMysql db;
    db.initialize("127.0.0.1","root","55555","test");
    QueryResult *queryResult=db.query("select * from student");
    return 0;
}
