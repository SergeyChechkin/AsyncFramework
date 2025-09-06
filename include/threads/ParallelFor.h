#pragma once

#include "common/macros.h"

#include <functional>
#include <thread>


void ParallelFor(
    size_t begin, 
    size_t end, 
    const std::function<void(size_t begin, size_t end)>& job, 
    size_t num_threads = std::thread::hardware_concurrency()) {
        
        const size_t size = end - begin;

        if UNLIKELY(num_threads < 2 || size < 2) {
            // No point to create a multiple threads 
            job(begin, end);
            return; 
        }

        // Compute tasks range for threads
        size_t interval = size / num_threads;
        size_t residual = size % num_threads;

        if UNLIKELY(interval < 1) {
            num_threads = size;
            interval = 1;
            residual = 0;
        }

        size_t job_begin = begin;
        size_t job_end;

        std::vector<std::thread> threads;

        while(job_begin < end) {
            // Evenly redestrebute residiual tasks 
            job_end = std::min(end, job_begin + interval);
            if (residual > 0) {
                ++job_end;
                --residual;
            }
            
            threads.push_back(std::thread(job, job_begin, job_end));
            
            job_begin = job_end;
        }

        for(size_t i = 0; i < num_threads; ++i) {
            threads[i].join();
        }
}