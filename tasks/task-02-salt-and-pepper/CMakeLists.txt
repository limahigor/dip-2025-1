cmake_minimum_required(VERSION 3.10)
project(SaltAndPepper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

add_executable(task2 ${CMAKE_SOURCE_DIR}/task-02-salt-and-pepper.cpp)

target_link_libraries(task2 ${OpenCV_LIBS})
