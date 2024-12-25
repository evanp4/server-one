cmake_minimum_required(VERSION 3.10)
project(MultithreadedWebServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.66 REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIRS} include)

add_executable(server src/server.cpp src/ThreadPool.cpp src/RequestQueue.cpp)
target_link_libraries(server ${Boost_LIBRARIES})
