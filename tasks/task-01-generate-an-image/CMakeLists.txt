cmake_minimum_required(VERSION 3.10)
project(GenerateAnImage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

add_executable(task1 ${CMAKE_SOURCE_DIR}/task-01-generate-an-image.cpp)

target_link_libraries(task1 ${OpenCV_LIBS})
