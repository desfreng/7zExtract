cmake_minimum_required(VERSION 3.26)
project(7zExtract)

set(CMAKE_CXX_STANDARD 17)

add_library(7zExtract
        7zLib/7zAlloc.c
        7zLib/7zBuf.c
        7zLib/7zFile.c
        7zLib/7zStream.c
        7zLib/7zCrc.c
        7zLib/7zArcIn.c
        7zLib/7zCrcOpt.c
        7zLib/7zDec.c
        7zLib/LzmaDec.c
        7zLib/Lzma2Dec.c
        7zLib/Bcj2.c
        7zLib/Delta.c
        7zLib/Bra.c
        7zLib/Bra86.c
        7zLib/CpuArch.c
        7zLib/7zBuf2.c
        7zLib/Ppmd7Dec.c
        7zLib/Ppmd7Enc.c
        7zLib/Ppmd7.c
        SevenZ.cpp
        SevenZ.h)

add_executable(7zExtractTester
        main.cpp)

target_link_libraries(7zExtractTester 7zExtract)