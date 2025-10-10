cmake_minimum_required(VERSION 3.10)
project(LoadImage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

add_executable(task5 ${CMAKE_SOURCE_DIR}/load_image_from_url.cpp)

target_link_libraries(task5 PRIVATE ${OpenCV_LIBS} CURL::libcurl)
