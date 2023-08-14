#include <stdexcept>
#include <cstring>
#include <codecvt>
#include <locale>
#include <fstream>

#include "SevenZ.h"
#include "7zLib/7zTypes.h"
#include "7zLib/7zAlloc.h"
#include "7zLib/7zCrc.h"
#include "7zLib/7zFile.h"
#include "7zLib/7z.h"

typedef struct FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

static void NtfsFileTime_to_FILETIME(const CNtfsFileTime *t, FILETIME *ft) {
    ft->dwLowDateTime = (DWORD) (t->Low);
    ft->dwHighDateTime = (DWORD) (t->High);
}

#define GET_TIME_64(pft) ((pft)->dwLowDateTime | ((UInt64)(pft)->dwHighDateTime << 32))

static const UInt32 kNumTimeQuantumsInSecond = 10000000;
static const UInt32 kFileTimeStartYear = 1601;
static const UInt32 kUnixTimeStartYear = 1970;
static const UInt64 kUnixTimeOffset =
        (UInt64) 60 * 60 * 24 * (89 + 365 * (kUnixTimeStartYear - kFileTimeStartYear));

static Int64 Time_FileTimeToUnixTime64(const FILETIME *ft) {
    const UInt64 winTime = GET_TIME_64(ft);
    return (Int64) (winTime / kNumTimeQuantumsInSecond) - (Int64) kUnixTimeOffset;
}

static const ISzAlloc g_Alloc = {SzAlloc, SzFree};
static const std::size_t kInputBufSize = 1 << 18;
static std::u16string fileNameBuffer;

static bool crcReady = false;
struct SevenZArchive::MyCFileInStream : public CFileInStream {
};
struct SevenZArchive::MyCLookToRead2 : public CLookToRead2 {
};
struct SevenZArchive::MyCSzArEx : public CSzArEx {
};

void SevenZArchive::check_error(int err) const {
    if (err == SZ_OK) {
        return;
    } else if (err == SZ_ERROR_MEM) {
        throw std::runtime_error("Error in memory management");
    } else if (err == SZ_ERROR_CRC) {
        throw std::runtime_error("CRC Error");
    } else if (err == SZ_ERROR_READ) {
        check_errno(err, "Read Error");
    } else {
        throw std::runtime_error("Unknown error. ID : " + std::to_string(err));
    }
}

void SevenZArchive::check_errno(int err, const std::string &res) const {
    if (err != 0) {
        if (archiveStream->wres != 0) {
            throw std::runtime_error(res + " " + std::strerror(archiveStream->wres));
        } else {
            throw std::runtime_error(res);
        }
    }
}

SevenZArchive::SevenZArchive(const std::string &path) : files() {
    archiveStream = std::make_unique<MyCFileInStream>();
    lookStream = std::make_unique<MyCLookToRead2>();
    db = std::make_unique<MyCSzArEx>();

    check_errno(InFile_Open(&archiveStream->file, path.c_str()), "Cannot open input file " + path);

    FileInStream_CreateVTable(archiveStream.get());
    archiveStream->wres = 0;
    LookToRead2_CreateVTable(lookStream.get(), False);

    lookStream->buf = static_cast<Byte *>(ISzAlloc_Alloc(&g_Alloc, kInputBufSize));
    if (!lookStream->buf)
        throw std::bad_alloc();
    else {
        lookStream->bufSize = kInputBufSize;
        lookStream->realStream = &archiveStream->vt;
        LookToRead2_INIT(lookStream.get())
    }


    if (!crcReady) {
        crcReady = true;
        CrcGenerateTable();
    }

    SzArEx_Init(db.get());
    check_error(SzArEx_Open(db.get(), &lookStream->vt, &g_Alloc, &g_Alloc));

    for (UInt32 i = 0; i < db->NumFiles; i++) {
        size_t len;

        if (SzArEx_IsDir(db.get(), i)) {
            continue;
        }

        len = SzArEx_GetFileNameUtf16(db.get(), i, nullptr);

        if (len > fileNameBuffer.size()) {
            fileNameBuffer.resize(len);
        }

        SzArEx_GetFileNameUtf16(db.get(), i, reinterpret_cast<UInt16 *>(fileNameBuffer.data()));
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        fileNameBuffer.erase(fileNameBuffer.find('\0'));

        Int64 timestamp;

        if (SzBitWithVals_Check(&db->MTime, i)) {
            FILETIME fileTime;
            NtfsFileTime_to_FILETIME(&db->MTime.Vals[i], &fileTime);
            timestamp = Time_FileTimeToUnixTime64(&fileTime);
        } else {
            timestamp = -1;
        }

        files.push_back(SevenZFileEntry(SzArEx_GetFileSize(db.get(), i), timestamp,
                                        convert.to_bytes(fileNameBuffer), i));
    }
}

SevenZArchive::~SevenZArchive() {
    SzArEx_Free(db.get(), &g_Alloc);
    ISzAlloc_Free(&g_Alloc, lookStream->buf);
    File_Close(&archiveStream->file);
}

void SevenZArchive::getData(const SevenZFileEntry &file, std::vector<unsigned char> &outBuffer) {
    static UInt32 blockIndex = 0;
    static std::vector<unsigned char> tempBuf;
    static size_t outBufferSize = 0;

    const UInt32 folderIndex = db->FileToFolder[file.fileIndex];

    if (folderIndex == (UInt32) -1) {
        tempBuf.clear();
        blockIndex = folderIndex;
        outBufferSize = 0;
        return;
    }

    if (tempBuf.empty() || blockIndex != folderIndex) {
        const UInt64 unpackSize = SzAr_GetFolderUnpackSize(&db->db, folderIndex);

        blockIndex = folderIndex;
        outBufferSize = unpackSize;
        tempBuf.resize(unpackSize);

        check_error(SzAr_DecodeFolder(&db->db, folderIndex, &lookStream->vt, db->dataPos,
                                      tempBuf.data(), unpackSize, &g_Alloc));
    }

    const UInt64 unpackPos = db->UnpackPositions[file.fileIndex];
    const size_t offset = unpackPos - db->UnpackPositions[db->FolderToFile[folderIndex]];

    if (offset + file.fileSize > outBufferSize) {
        check_error(SZ_ERROR_FAIL);
    }
    if (SzBitWithVals_Check(&db->CRCs, file.fileIndex)) {
        if (CrcCalc(tempBuf.data() + offset, file.fileSize) != db->CRCs.Vals[file.fileIndex]) {
            check_error(SZ_ERROR_CRC);
        }
    }

    outBuffer = std::vector<unsigned char>(&tempBuf[offset], &tempBuf[offset + file.fileSize]);
}

void SevenZArchive::extractInFile(const SevenZFileEntry &file, const fs::path &path) {
    std::vector<unsigned char> data;
    getData(file, data);
    std::ofstream file_stream(path, std::ios::out | std::ios::binary);
    std::copy(data.cbegin(), data.cend(), std::ostreambuf_iterator<char>(file_stream));
    file_stream.flush();
    file_stream.close();
}

unsigned int SevenZArchive::nbFiles() {
    return files.size();
}
