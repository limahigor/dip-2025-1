cmake_minimum_required(VERSION 3.10)
project(FilteringSPNoise)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

add_executable(task3 ${CMAKE_SOURCE_DIR}/task-03-filtering-sp-noise.cpp)

target_link_libraries(task3 ${OpenCV_LIBS})
