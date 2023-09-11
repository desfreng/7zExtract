cmake_minimum_required(VERSION 3.20)
project(7zExtract)

add_subdirectory(lzma-sdk)
add_library(7zExtract SevenZ.cpp SevenZ.h)

target_include_directories(7zExtract PRIVATE lzma-sdk)
target_link_libraries(7zExtract PRIVATE lzma-sdk)

set_property(TARGET 7zExtract PROPERTY CXX_STANDARD 17)
set_property(TARGET 7zExtract PROPERTY CXX_STANDARD_REQUIRED ON)

add_executable(7zExtractTester main.cpp)
target_link_libraries(7zExtractTester 7zExtract)