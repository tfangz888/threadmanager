// XXXMsgHandler.h

#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include "Message.h"          // 包含Message结构体的定义

class ThreadSafeQueue;
class XXXMsgHandler {
#define GETTID() syscall(SYS_gettid)
private:
    std::atomic<pid_t> thread_id_; // threadID，短ID
    int worker_id_; // 建立的thread的序号，从0开始
    std::vector<ThreadSafeQueue> message_queues_;  // 5个消息队列
    std::atomic<bool> running_;
    std::thread worker_thread_;
    
    // 统计信息
    std::atomic<int> processed_messages_;

    // 消息处理主循环
    void processMessages();

    // 处理单条消息
    void processMessage(std::shared_ptr<Message> msg);

public:
    XXXMsgHandler(int worker_id);
    ~XXXMsgHandler();

    // 启动处理线程
    void start();

    // 停止处理线程
    void stop();

    // 添加消息到指定队列
    bool addMessage(std::shared_ptr<Message> msg);

    // 获取线程ID
    pid_t getThreadId() const;

    // 获取工作线程ID
    int getWorkerId() const;

    // 获取队列状态
    void getQueueStatus() const;
};


