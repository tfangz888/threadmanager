// IXXXThreadManager.h

#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <string>

#include "Message.h"

class IXXXThreadManager {
public:
    virtual ~IXXXThreadManager() {}

    virtual void startAll() = 0;
    virtual void stopAll() = 0;
    virtual bool addMessage(int worker_id, std::shared_ptr<Message> msg) = 0;
    virtual bool addMessageBalanced(std::shared_ptr<Message> msg) = 0;
    virtual int getThreadCount() const = 0;
    virtual const std::vector<pid_t>& getThreadIds() const = 0;
    virtual void showAllQueuesStatus() const = 0;
};


