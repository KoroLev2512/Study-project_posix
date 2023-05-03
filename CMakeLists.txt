cmake_minimum_required(VERSION 3.16)
project(producer_consumer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(producer_consumer producer_consumer.cpp)

target_link_libraries(producer_consumer PRIVATE pthread)
