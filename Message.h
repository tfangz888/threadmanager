// Message.h

#pragma once

#include <string>
#include <iostream>
#include <iomanip>

struct Message {
    int id;
    std::string data;
    int priority;  // 0-4 对应5个队列的优先级
    
    Message(int _id, const std::string& _data, int _priority = 0)
        : id(_id), data(_data), priority(_priority) {}
};


