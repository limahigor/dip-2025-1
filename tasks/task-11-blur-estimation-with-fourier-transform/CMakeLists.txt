set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.10)
project(FFT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

add_executable(task11 ${CMAKE_SOURCE_DIR}/task-11-blur-estimation-with-fourier-transform.cpp)

target_link_libraries(task11 ${OpenCV_LIBS})
