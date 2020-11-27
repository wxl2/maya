//
// Created by wxl on 2020/11/25.
//

#include "DaemonRun.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void maya::detail::daemon_run()
{
    pid_t pid;
    //忽略子进程产生的信号
    signal(SIGCHLD,SIG_IGN);
    //1）在父进程中，fork返回新创建子进程的进程ID；
    //2）在子进程中，fork返回0；
    //3）如果出现错误，fork返回一个负值；
    pid=fork();
    if(pid<0)
    {
        printf("fork error\n");
        exit(-1);
    }
    else if(pid>0)
    {
        //父进程退出,子进程独立运行
        exit(0);
    }
    //之前parent和child运行在同一个session里,parent是会话（session）的领头进程,
    //parent进程作为会话的领头进程，如果exit结束执行的话，那么子进程会成为孤儿进程，并被init收养。
    //执行setsid()之后,child将重新获得一个新的会话(session)id。
    //这时parent退出之后,将不会影响到child了。
    setsid();
    int fd;
    fd=open("/dev/null",O_RDWR,0);
    if(fd!=-1)
    {
        //dup2将fd文件描述符号复制给STDIN_FILENO,但由于STDIN_FILENO已经被打开
        //所以会先将STDIN_FILENO关闭而后将fd复制给其
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);//将fd指向0,1,2文件描述符号
    }
    if(fd>2)
        close(fd);//重定向之后便可以关闭fd
}