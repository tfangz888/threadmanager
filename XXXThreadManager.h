// XXXThreadManager.h

#pragma once


#include <vector>
#include <memory>
#include <atomic>
#include <string>

#include "IXXXThreadManager.h"
#include "XXXMsgHandler.h"

class XXXThreadManager : public IXXXThreadManager {
private:
    std::vector<std::unique_ptr<XXXMsgHandler>> handlers_;
    std::vector<pid_t> thread_ids_;
    int thread_count_;

public:
    // 构造函数和析构函数
    XXXThreadManager();
    ~XXXThreadManager();

    // 实现接口方法
    void startAll() override;
    void stopAll() override;
    bool addMessage(int worker_id, std::shared_ptr<Message> msg) override;
    bool addMessageBalanced(std::shared_ptr<Message> msg) override;
    int getThreadCount() const override;
    const std::vector<pid_t>& getThreadIds() const override;
    void showAllQueuesStatus() const override;
};


