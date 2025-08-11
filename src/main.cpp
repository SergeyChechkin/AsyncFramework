#include <eigen3/Eigen/Core>

#include "../test/ThreadPoolTest.h"

#include <iostream>

int main() {
    NoParallelForTest();
    ParallelForFuncTest();
    TreadPoolTest();

    return 0;
}