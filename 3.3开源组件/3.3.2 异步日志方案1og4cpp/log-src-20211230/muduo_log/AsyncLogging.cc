// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <stdio.h>


AsyncLogging::AsyncLogging(const string& basename,
                           off_t rollSize,
                           int flushInterval)
  : flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    rollSize_(rollSize),
    thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    latch_(1),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

int append_cnt = 0;
void AsyncLogging::append(const char* logline, int len)
{
  // if(cnt++ == 50000)abort();
  MutexLockGuard lock(mutex_);    // 多线程加锁
  if (currentBuffer_->avail() > len)    // 判断buffer还有没有空间写入这条日志
  {
    currentBuffer_->append(logline, len); // 直接写入
  }
  else
  {
    buffers_.push_back(std::move(currentBuffer_));    // buffers_是vector，把buffer入队列
    // printf("push_back append_cnt:%d, size:%d\n", ++append_cnt, buffers_.size());
    if (nextBuffer_)  // 用了双缓存
    {
      currentBuffer_ = std::move(nextBuffer_);    // 如果不为空则将buffer转移到currentBuffer_
    }
    else
    {
      // 重新分配buffer
      currentBuffer_.reset(new Buffer); // Rarely happens如果后端写入线程没有及时读取数据，那要再分配buffer
    }
    currentBuffer_->append(logline, len);   // buffer写满了
    cond_.notify(); // 唤醒写入线程
  }
}
int threadFunc_cnt = 0;
void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  latch_.countDown();
  LogFile output(basename_, rollSize_, false);
  BufferPtr newBuffer1(new Buffer); // 是给currentBuffer_
  BufferPtr newBuffer2(new Buffer); // 是给nextBuffer_
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;    // 保存要写入的日志
  buffersToWrite.reserve(16);
  while (running_)
  {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    { // 锁的作用域
      MutexLockGuard lock(mutex_);
      if (buffers_.empty())  // 没有数据可读取，休眠
      {
        // printf("waitForSeconds into\n");
        cond_.waitForSeconds(flushInterval_);   // 超时退出或者被唤醒（收到notify）
        // printf("waitForSeconds leave\n");
      }
      buffers_.push_back(std::move(currentBuffer_));  // currentBuffer_被锁住  currentBuffer_被置空
      // printf("push_back threadFunc:%d, size:%d\n", ++threadFunc_cnt, buffers_.size());
      currentBuffer_ = std::move(newBuffer1); // currentBuffer_ 需要内存空间
      buffersToWrite.swap(buffers_);          // 用了双队列，把前端日志的队列所有buffer都转移到buffersToWrite队列
      if (!nextBuffer_)     // newBuffer2是给nextBuffer_
      {
        nextBuffer_ = std::move(newBuffer2);  // 如果为空则使用newBuffer2的缓存空间
      }
    }
    // 从这里是没有锁，数据落盘的时候不要加锁
    assert(!buffersToWrite.empty());
    // fixme的操作 4M一个buffer *25 = 100M
    if (buffersToWrite.size() > 25)  // 这里缓存的数据太多了，比如4M为一个buffer空间，25个buffer就是100M了。
    {
      printf("Dropped\n");
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffersToWrite.size()-2);    // 只保留2个buffer
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());   // 只保留2个buffer(默认4M)
    }

    for (const auto& buffer : buffersToWrite)  // 遍历buffer
    {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.append(buffer->data(), buffer->length());    // 负责fwrite数据
    }
    output.flush();   // 保证数据落到磁盘了
    if (buffersToWrite.size() > 2)
    {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);   // 只保留2个buffer
    }

    if (!newBuffer1)
    {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());    // 复用buffer对象
      buffersToWrite.pop_back();
      newBuffer1->reset();    // 重置
    }

    if (!newBuffer2)
    {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());   // 复用buffer对象
      buffersToWrite.pop_back();
      newBuffer2->reset();   // 重置
    }

    buffersToWrite.clear(); 
    
  }
  output.flush();
}

