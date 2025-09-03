#pragma once

#include "common/macros.h"

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <queue>
#include <iostream>

class SingleThread {
public:
    SingleThread() {
        thread_ = std::thread([this] {
            while(true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    cv_.wait(lock, [this] {return !tasks_.empty() || STOP == state_ || FINISH_AND_STOP == state_;});
                    if (STOP == state_ || (tasks_.empty() && FINISH_AND_STOP == state_)) {
                        break;
                    }
                    
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task();
            }   
        }); 
    }

    ~SingleThread() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // TODO: add configuration to handle both cases
            //state_ = STOP;
            state_ = FINISH_AND_STOP;
        }

        cv_.notify_one();

        thread_.join();
    }

    void addTask(std::function<void()> task) noexcept {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::move(task));
        }

        cv_.notify_one();
    }
private:
    std::thread thread_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    
    enum State {
        RUN,
        STOP,
        FINISH_AND_STOP,
    };

    State state_ = RUN;
};