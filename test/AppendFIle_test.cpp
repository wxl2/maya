//
// Created by wxl on 2020/10/12.
//

#include "../base/FileUtil.h"
#include <thread>


int main()
{
    std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    maya::FileUtil::AppendFile file("file");
    for(int i=0;i<1025;i++)
    {
        file.append(line.c_str(),line.length());
    }
    return 0;
}

