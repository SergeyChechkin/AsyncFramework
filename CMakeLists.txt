cmake_minimum_required(VERSION 3.1)
project(async_fw)

set(CMAKE_CXX_STANDARD 20)

find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/AsyncNode.cpp)
    
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${EIGEN3_LIBS})