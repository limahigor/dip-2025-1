set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.10)
project(HistogramMatching)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

add_executable(task10 ${CMAKE_SOURCE_DIR}/task-07-histogram-matching.cpp)

target_link_libraries(task10 ${OpenCV_LIBS})
