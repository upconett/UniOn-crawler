cmake_minimum_required(VERSION 3.10)
project(TDLightTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# TDLight
set(Td_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/tdlight/tdlight/lib/cmake/Td")
find_package(Td 1.8.35 REQUIRED)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE
    Td::TdStatic
)
