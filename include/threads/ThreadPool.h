#pragma once

#include "common/macros.h"

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <queue>
#include <iostream>

class ThreadPool {
public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        ASSERT(num_threads > 0, "Invalid number of threads.");
        for(unsigned int i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this] {
                    while(true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(queue_mutex_);
                            cv_.wait(lock, [this] {return !tasks_.empty() || stop_;});
                            if (stop_) {
                                break;
                            }
                            
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }

                        task();
                    }   
            }); // threads_.emplace_back
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }

        cv_.notify_all();

        for(auto& thread : threads_) {
            thread.join();
        }
    }

    void enqueue(std::function<void()> task) noexcept {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::move(task));
        }

        cv_.notify_one();
    }

    void ParallelFor(
        int begin, 
        int end, 
        const std::function<void(int begin, int end)>& job) {
        ASSERT(end >= begin, "Invalid tasks range.");

        int num_threads = threads_.size();
        const int size = end - begin;

        // Execute a single job in this thread.
        if UNLIKELY(num_threads < 2 || size < 2) {
            job(begin, end);
            return; 
        }

        // Estimate jobs ranges 
        int interval = size / num_threads;
        int residual = size % num_threads;

        if UNLIKELY(interval < 1) {
            num_threads = size;
            interval = 1;
            residual = 0;
        }

        int job_begin = begin;
        int job_end;

        std::counting_semaphore cs_done(0); 

        // queue jobs
        int jobs_count = 0;
        while(job_begin < end) {
            job_end = std::min(end, job_begin + interval);
            if (residual > 0) {
                ++job_end;
                --residual;
            }
            
            enqueue([=, &cs_done]{job(job_begin, job_end); cs_done.release();});
            job_begin = job_end;
            ++jobs_count;
        }

        // wait for jobs to get done 
        for(size_t i = 0; i < jobs_count; ++i) {
            cs_done.acquire();
        }
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};