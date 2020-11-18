//
// Created by wxl on 2020/10/13.
//

#include "base/LogFile.h"
#include <string>



int main()
{
    std::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    maya::LogFile file("file",10000);

    //printf("%s\n",maya::strerror_tl(1));
    for(int i=0;i<10000;i++)
    {
        file.append(line.c_str(),line.length());
        usleep(1000);
    }
    return 0;
}
