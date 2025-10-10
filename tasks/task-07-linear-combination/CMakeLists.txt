cmake_minimum_required(VERSION 3.10)
project(ImageSimilarity)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

add_executable(task7 ${CMAKE_SOURCE_DIR}/linear_combination.cpp)

target_link_libraries(task7 ${OpenCV_LIBS})
