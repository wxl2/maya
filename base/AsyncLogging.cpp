//
// Created by wxl on 2020/10/14.
//

#include "AsyncLogging.h"
#include "LogFile.h"
#include <chrono>

using namespace maya;
using namespace std::chrono_literals;//since C++14

std::unique_ptr<std::thread> AsyncLogging::thread_;
std::mutex AsyncLogging::mutex_;
std::condition_variable AsyncLogging::cond_;



AsyncLogging::AsyncLogging(const std::string &basename, off_t rollSize,int flushInterval)
:flushInterval_(flushInterval),
 running_(false),
 basename_(basename),
 rollSize_(rollSize),
 latch_(1),
 currentBuffer_(new Buffer),
 nextBuffer_(new Buffer),
 buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char *logline, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));

        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}

void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    //FIXME 此处使用while循环，buffers_为空，则有可能无法将AsyncLogging缓冲区内容append到LogFile缓冲区，到是fflush刷新为空，
    //FIXME 若currentBuffer_的内容还未满，append无法将其添加只buffers_,而running_被置false，无法执行循环体，则无法将其添加至LogFile缓冲区
    //TODO: 修改循环体
    //while (running_)
    do
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty())  // 此处不能使用while，否则join会一直等待，因为退出是empty始终为真，[FIXME:不是平常用法]
            {
                //printf("wait\n"); //for debug
                cond_.wait_for(lock,flushInterval_*1ms);
            }
            buffers_.push_back(std::move(currentBuffer_));//FIXME:that
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toFormattedString().c_str(),
                     buffersToWrite.size()-2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }while (running_);
    output.flush();
}
