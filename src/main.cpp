#include <eigen3/Eigen/Core>

#include "../test/ThreadPoolTest.h"
#include "../test/AsyncNodeTest.h"

#include <iostream>

int main() {
    //NoParallelForTest();
    //ParallelForFuncTest();
    //TreadPoolTest();

    AsyncNodeTest();

    return 0;
}