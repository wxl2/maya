//
// Created by wxl on 2020/10/18.
//

#include "../base/ThreadPool.h"
#include "../base/CountDownLatch.h"
#include "../base/Logging.h"

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>

using namespace std::chrono_literals;
void print()
{
    std::cout<<std::this_thread::get_id()<<std::endl;
}

void printString(const std::string& str)
{
    LOG_INFO << str;
    //usleep(100*1000);
}

void test(int maxSize)
{
    LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    maya::ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    LOG_WARN << "Adding";
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(std::bind(printString, std::string(buf)));
    }
    LOG_WARN << "Done";
    maya::CountDownLatch latch(1);
    pool.run(std::bind(&maya::CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
}

/*
 * Wish we could do this in the future.
void testMove()
{
  muduo::ThreadPool pool;
  pool.start(2);

  std::unique_ptr<int> x(new int(42));
  pool.run([y = std::move(x)]{ printf("%d: %d\n", muduo::CurrentThread::tid(), *y); });
  pool.stop();
}
*/

void longTask(int num)
{
    LOG_INFO << "longTask " << num;
    std::this_thread::sleep_for(std::chrono::microseconds(2));
}

void test2()
{
    LOG_WARN << "Test ThreadPool by stoping early.";
    maya::ThreadPool pool("ThreadPool");
    pool.setMaxQueueSize(5);
    pool.start(3);

    std::thread thread1([&pool]()
                          {
                              for (int i = 0; i < 20; ++i)
                              {
                                  pool.run(std::bind(longTask, i));
                              }
                          });

    std::this_thread::sleep_for(std::chrono::microseconds(2));
    LOG_WARN << "stop pool";
    pool.stop();  // early stop

    thread1.join();
    // run() after stop()
    pool.run(print);
    LOG_WARN << "test2 Done";
}

int main()
{
//    test(0);
//    test(1);
//    /test(5);
//    test(10);
//    test(50);
    test2();
}


