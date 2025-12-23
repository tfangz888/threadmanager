#include "XXXThreadManager.h"

XXXThreadManager::XXXThreadManager() {
    // 获取CPU核心数
    thread_count_ = std::thread::hardware_concurrency();
    if (thread_count_ == 0) {
        thread_count_ = 4;  // 默认4个线程
    }
    
    std::cout << "Creating " << thread_count_ << " worker threads..." << std::endl;
    
    // 创建线程处理器
    handlers_.reserve(thread_count_);
    thread_ids_.reserve(thread_count_);
    
    for (int i = 0; i < thread_count_; ++i) {
        handlers_.push_back(std::make_unique<XXXMsgHandler>(i));
    }
}

XXXThreadManager::~XXXThreadManager() {
    stopAll();
}

void XXXThreadManager::startAll() {
    for (auto& handler : handlers_) {
        handler->start();
        thread_ids_.push_back(handler->getThreadId());
    }
    
    std::cout << "All threads started. Thread IDs:" << std::endl;
    for (size_t i = 0; i < thread_ids_.size(); ++i) {
        std::cout << "  Worker " << i << ": " << thread_ids_[i] << std::endl;
    }
}

void XXXThreadManager::stopAll() {
    for (auto& handler : handlers_) {
        handler->stop();
    }
    thread_ids_.clear();
}

bool XXXThreadManager::addMessage(int worker_id, std::shared_ptr<Message> msg) {
    if (worker_id >= 0 && worker_id < thread_count_) {
        return handlers_[worker_id]->addMessage(msg);
    }
    return false;
}

bool XXXThreadManager::addMessageBalanced(std::shared_ptr<Message> msg) {
    static std::atomic<int> round_robin_counter(0);
    int worker_id = round_robin_counter++ % thread_count_;
    return addMessage(worker_id, msg);
}

int XXXThreadManager::getThreadCount() const {
    return thread_count_;
}

const std::vector<pid_t>& XXXThreadManager::getThreadIds() const {
    return thread_ids_;
}

void XXXThreadManager::showAllQueuesStatus() const {
    std::cout << "\n=== All Workers Status ===" << std::endl;
    for (const auto& handler : handlers_) {
        handler->getQueueStatus();
        std::cout << std::endl;
    }
}
