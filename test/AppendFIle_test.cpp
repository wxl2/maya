//
// Created by wxl on 2020/10/12.
//

#include "../base/FileUtil.h"
#include <thread>

void func(){
        maya::FileUtil::AppendFile file1("file1");
};

void func2(){
    maya::FileUtil::AppendFile file1("file");
};
int main()
{
    std::thread th1(func);
    std::thread th2(func2);
    th1.join();
    th2.join();
    return 0;
}

