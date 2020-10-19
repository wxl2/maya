//
// Created by wxl on 2020/10/18.
//

#include "base/CircularList.h"
#include <iostream>
int main()
{
    CricularList<int> test(3);
    try
    {
        for(int i=0;i<3;i++)
        {
            test.push_back(i);
            std::cout<<i<<std::endl;
        }
    }
    catch (illegalParameterValue err)
    {
        err.outputMessage();
    }
    std::cout<<test.front()<<std::endl;
    std::cout<<test.back()<<std::endl;
    std::cout<<test.size()<<std::endl;
    std::cout<<test.empty()<<std::endl;
    std::cout<<test.full()<<std::endl;

    return 0;
}
