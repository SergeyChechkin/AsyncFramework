#pragma once

#include <threads/ParallelFor.h>
#include <threads/ThreadPool.h>

// test includes 
#include <iostream>
#include <vector>
#include <thread>
#include <x86intrin.h> // gcc specific, for __rdtsc() - CPU counter

const size_t size = 1000;
std::vector<int> result(size, 0);

int test_func(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return x + x;
}


void NoParallelForTest() {
    auto begin = __rdtsc(); 
    for(size_t i = 0; i < size; ++i) {
        result[i] = test_func(i);
    }
    auto end = __rdtsc();
    std::cout << "NoParallelFor - " << (end - begin) / size << std::endl;
} 

void ParallelForFuncTest() {

    auto thread_body = [&](size_t range_begin, size_t range_end) {
        while(range_begin < range_end) {
            result[range_begin] = test_func(range_begin);
            ++range_begin;
        }
    };

    auto begin = __rdtsc();
    ParallelFor(0, size, thread_body);
    auto end = __rdtsc();
    std::cout << "ParallelFor function - " << (end - begin) / size << std::endl;
} 

void TreadPoolTest() {
    ThreadPool pool;

    auto thread_body = [](size_t range_begin, size_t range_end) {
        while(range_begin < range_end) {
            result[range_begin] = test_func(range_begin);
            ++range_begin;
        }
    };

    auto begin = __rdtsc();
    pool.ParallelFor(0, size, thread_body);
    auto end = __rdtsc();
    std::cout << "ParallelFor pool - " << (end - begin) / size << std::endl;
}