// XXXMsgHandler.cpp


#include <iostream>
#include <sys/syscall.h>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>
#include <pthread.h>

#include "XXXMsgHandler.h"

namespace {
    // 设置线程亲和性
    void setThreadAffinity(cpu_set_t *cpuset, int cpu, pthread_t thread) {
      if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), cpuset) != 0) {
        std::cerr << "Error setting thread affinity" << std::endl;
      }
    }
}

// 线程安全的消息队列
class ThreadSafeQueue {
private:
    std::queue<std::shared_ptr<Message>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    
public:
    void push(std::shared_ptr<Message> msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        condition_.notify_one();
    }
    
    bool pop(std::shared_ptr<Message>& msg, int timeout_ms = 100) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (condition_.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                               [this] { return !queue_.empty(); })) {
            msg = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

/******************************************************************************/

XXXMsgHandler::XXXMsgHandler(int worker_id)
    : thread_id_(0), worker_id_(worker_id), message_queues_(5), running_(false), processed_messages_(0) {
}

XXXMsgHandler::~XXXMsgHandler() {
    stop();
}

void XXXMsgHandler::start() {
    if (running_) return;
    running_ = true;
    worker_thread_ = std::thread(&XXXMsgHandler::processMessages, this);
    
    std::cout << "Worker " << worker_id_ 
              << " started with thread ID: " << thread_id_ 
              << std::endl;
}

void XXXMsgHandler::stop() {
    if (running_) {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        std::cout << "Worker " << worker_id_ << " stopped. "
                  << "Processed " << processed_messages_ << " messages." << std::endl;
    }
}

bool XXXMsgHandler::addMessage(std::shared_ptr<Message> msg) {
    if (!running_ || msg->priority < 0 || msg->priority >= 5) {
        return false;
    }
    
    message_queues_[msg->priority].push(msg);
    return true;
}

pid_t XXXMsgHandler::getThreadId() const {
    return thread_id_;
}

int XXXMsgHandler::getWorkerId() const {
    return worker_id_;
}

void XXXMsgHandler::getQueueStatus() const {
    std::cout << "Worker " << worker_id_ << " Queue Status:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "  Queue " << i << ": " << message_queues_[i].size() 
                  << " messages" << std::endl;
    }
    std::cout << "  Total processed: " << processed_messages_ << std::endl;
}

// 消息处理主循环
void XXXMsgHandler::processMessages() {
    // 设置线程亲和性
    int cpu_count = std::thread::hardware_concurrency();
    if (cpu_count > 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        int target_cpu = worker_id_ % cpu_count;
        CPU_SET(target_cpu, &cpuset);
        setThreadAffinity(&cpuset, target_cpu, worker_thread_.native_handle());
    }
    std::this_thread::yield();// 让调度器重新调度，以使本线程绑定到设置的CPU core
        
    // 在工作线程内部获取真正的TID
    thread_id_ = GETTID();
    int current_cpu = getCurrentCPU();
    
    std::cout << "Worker " << worker_id_ 
              << " process started - TID: " << thread_id_ 
              << ", CPU: " << current_cpu << std::endl;
              
    while (running_) {
        bool found_message = false;
        
        for (int priority = 0; priority < 5; ++priority) {
            std::shared_ptr<Message> msg;
            if (message_queues_[priority].pop(msg, 10)) {  // 10ms超时
                processMessage(msg);
                found_message = true;
                break;  // 处理完一条消息后重新从最高优先级开始
            }
        }
        
        if (!found_message) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void XXXMsgHandler::processMessage(std::shared_ptr<Message> msg) {
    std::cout << "Worker " << worker_id_ 
              << " processing message ID: " << msg->id 
              << ", Priority: " << msg->priority 
              << ", Data: " << msg->data << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    processed_messages_++;
}
