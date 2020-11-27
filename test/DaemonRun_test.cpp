//
// Created by wxl on 2020/11/25.
//

#include "utils/DaemonRun.h"

int main()
{
    maya::detail::daemon_run();
    while(1);
    return 0;
}