CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

ADD_LIBRARY(lzma-sdk 7zAlloc.c 7zAlloc.h 7zArcIn.c 7zBuf2.c 7zBuf.c 7zBuf.h 7zCrc.c 7zCrc.h
            7zCrcOpt.c 7zDec.c 7zFile.c 7zFile.h 7z.h 7zStream.c 7zTypes.h 7zVersion.h 7zWindows.h
            Alloc.c Alloc.h Bcj2.c Bcj2Enc.c Bcj2.h Bra86.c Bra.c Bra.h BraIA64.c Compiler.h
            CpuArch.c CpuArch.h Delta.c Delta.h DllSecur.c DllSecur.h LzFind.c LzFind.h LzFindMt.c
            LzFindMt.h LzFindOpt.c LzHash.h Lzma2Dec.c Lzma2Dec.h Lzma2DecMt.c Lzma2DecMt.h
            Lzma2Enc.c Lzma2Enc.h Lzma86Dec.c Lzma86Enc.c Lzma86.h LzmaDec.c LzmaDec.h LzmaEnc.c
            LzmaEnc.h LzmaLib.c LzmaLib.h MtCoder.c MtCoder.h MtDec.c MtDec.h Ppmd7.c Ppmd7Dec.c
            Ppmd7Enc.c Ppmd7.h Ppmd.h Precomp.h RotateDefs.h Sort.c Sort.h SwapBytes.c SwapBytes.h
            Threads.c Threads.h)

IF (LZMA_WITH_AES)
    MESSAGE("Building lzma-sdk with AES Support")
    TARGET_SOURCES(lzma-sdk PRIVATE Aes.c Aes.h AesOpt.c)
ELSE ()
    MESSAGE("lzma-sdk: Dropping AES Support")
ENDIF ()


IF (LZMA_WITH_SHA)
    MESSAGE("Building lzma-sdk with SHA Support")
    TARGET_SOURCES(lzma-sdk PRIVATE Sha256.c Sha256.h Sha256Opt.c)
ELSE ()
    MESSAGE("lzma-sdk: Dropping SHA Support")
ENDIF ()


IF (LZMA_WITH_XZ)
    MESSAGE("Building lzma-sdk with Xz Support")
    TARGET_SOURCES(lzma-sdk PRIVATE Xz.c XzCrc64.c XzCrc64.h XzCrc64Opt.c XzDec.c XzEnc.c XzEnc.h Xz.h XzIn.c)
ELSE ()
    MESSAGE("lzma-sdk: Dropping Zx Support")
ENDIF ()

SET_PROPERTY(TARGET lzma-sdk PROPERTY C_STANDARD 99)
SET_PROPERTY(TARGET lzma-sdk PROPERTY CMAKE_C_STANDARD_REQUIRED ON)
