#pragma once

#include "common/macros.h"

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>

#include <queue>
#include <iostream>

class ThreadPool {
public:
    void Start(size_t num_threads = std::thread::hardware_concurrency()) noexcept {
        ASSERT(num_threads > 0, "Invalid number of threads.");
        
        state_ = RUN;
        threads_.reserve(num_threads);
        for(unsigned int i = 0; i < num_threads; ++i) {
            threads_.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
        }
    }

    void Stop() noexcept {
        // Notify all threads to stop  
        state_ = STOP;
        mutex_condition_.notify_all();

        // Join all threads
        for(auto& thread : threads_) {
            thread.join();
        }

        threads_.clear();
    }

    void Pause() noexcept {
        // Notify all threads to pause 
        state_ = PAUSE;
        mutex_condition_.notify_all();
    }

    void Resume() noexcept {
        // Notify all threads to run
        state_ = RUN;
        mutex_condition_.notify_all();
    }

    void QueueJob(const std::function<void()>& job) noexcept {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            jobs_.push(job);
        }

        mutex_condition_.notify_one();
    }
    
    // Run parallel computation on pool threads. 
    // Blocked, until jobs are done.    
    void ParallelFor(
        size_t begin, 
        size_t end, 
        const std::function<void(size_t begin, size_t end)>& job) {
        
        auto num_threads = threads_.size();
        const size_t size = end - begin;

        // Execute a single job in this thread.
        if UNLIKELY(num_threads < 2 || size < 2) {
            job(begin, end);
            return; 
        }

        // Estimate jobs ranges 
        size_t interval = size / num_threads;
        size_t residual = size % num_threads;

        if UNLIKELY(interval < 1) {
            num_threads = size;
            interval = 1;
            residual = 0;
        }

        size_t job_begin = begin;
        size_t job_end;

        std::counting_semaphore smphr(0); 

        auto thread_body = [&](size_t begin, size_t end) {
            job(begin, end);
            smphr.release();
        };

        // queue jobs
        int jobs_count = 0;
        while(job_begin < end) {
            job_end = std::min(end, job_begin + interval);
            if (residual > 0) {
                ++job_end;
                --residual;
            }
            QueueJob([&]{thread_body(job_begin, job_end);});
            job_begin = job_end;
            ++jobs_count;
        }

        // wait for jobs to get done 
        for(size_t i = 0; i < jobs_count; ++i) {
            smphr.acquire();
        }
    }
private:
    // actual threads
    std::vector<std::thread> threads_;
    // state of the thread pool 
    enum State {
        UNKNOWN = 0,
        STOP = 1,
        PAUSE = 2,
        RUN = 3,
    };
    std::atomic<int> state_ = UNKNOWN;
    // interthread comunication 
    std::mutex notification_mutex_;
    std::condition_variable mutex_condition_;
    // jobs queue
    std::queue<std::function<void()>> jobs_;
    std::mutex queue_mutex_;

    void ThreadLoop() noexcept {
        while(state_ != STOP) {   
            // Wait for notification
            std::unique_lock<std::mutex> lock(notification_mutex_); // TODO: use the queue mutex ?
            mutex_condition_.wait(lock, [this] {
                return 
                    (!jobs_.empty() && state_ == RUN)   // we have a job to do
                    || state_ == STOP;                  // we need to stop the thred
            });

            // Execute a job from the queue
            std::function<void()> job = nullptr;
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (!jobs_.empty()) {
                    job = jobs_.front();
                    jobs_.pop(); 
                }
            }
            
            if (job)
                job();
        }
    }
};