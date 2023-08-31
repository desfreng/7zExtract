cmake_minimum_required(VERSION 3.20)

file(GLOB LZMA_SRC "*.h" "*.c")
add_library(lzma-sdk ${LZMA_SRC})

set_property(TARGET lzma-sdk PROPERTY C_STANDARD 99)
set_property(TARGET lzma-sdk PROPERTY CMAKE_C_STANDARD_REQUIRED ON)
