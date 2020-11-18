//
// Created by wxl on 2020/10/12.
//

#include "base/FileUtil.h"
#include "base/LogFile.h"
#include <functional>
#include <unistd.h>
#include <thread>
std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
//void func(maya::FileUtil::AppendFile *file_)
//{
//    for(int i=0;i<2;++i)
//    file_->append(line.c_str(),line.length());
//}
//int main()
//{
//
//    maya::FileUtil::AppendFile file("file");
//    std::thread th(std::bind(func,&file));
//    return 0;
//}

#include<thread>
#include <memory>
#include <functional>
class AsyncLogging
{
public:
    AsyncLogging(){}
    void start()
    {
        thread_.reset(new std::thread(std::bind(&AsyncLogging::threadFunc,this)));
    }

    void stop()
    {
        if(thread_->joinable())
            thread_->join();
    }
private:
    void threadFunc();
    static std::unique_ptr<std::thread> thread_;
};
std::unique_ptr<std::thread> AsyncLogging::thread_;
void  AsyncLogging::threadFunc()
{

    maya::LogFile output("file",  500*1000*1000, false);
    output.append(line.c_str(),line.length());
    output.flush();
}
int main(){
    AsyncLogging test;
    test.start();
    test.stop();
    while(1);
    return 0;
}
