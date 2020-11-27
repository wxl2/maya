//
// Created by wxl on 2020/11/25.
// 让程序以守护进程方式运行
//

#ifndef MAYA_DAEMONRUN_H
#define MAYA_DAEMONRUN_H

namespace maya{
    namespace detail{
        void daemon_run();
    }//namespace detail
}//namespace maya

#endif //MAYA_DAEMONRUN_H
