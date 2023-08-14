#ifndef INC_7Z_EXTRACTOR_SEVEN_Z_ARCHIVE_H
#define INC_7Z_EXTRACTOR_SEVEN_Z_ARCHIVE_H


#include <string>
#include <filesystem>
#include <functional>
#include <utility>

namespace fs = std::filesystem;

class SevenZArchive;

class SevenZFileEntry {
public:
    [[maybe_unused]] const std::size_t fileSize;
    [[maybe_unused]] const std::time_t creationTime;
    [[maybe_unused]] const fs::path fileName;

protected:
    SevenZFileEntry(const std::size_t fileSize,
                    const std::time_t creationTime,
                    fs::path fileName,
                    const unsigned int fileIndex) : creationTime(creationTime),
                                                    fileIndex(fileIndex),
                                                    fileSize(fileSize),
                                                    fileName(std::move(fileName)) {}

    const unsigned int fileIndex;

    friend class SevenZArchive;
};


class SevenZArchive {
public:
    explicit SevenZArchive(const std::string &path);

    virtual ~SevenZArchive();

    void extractInFile(const SevenZFileEntry &file, const fs::path &path);

    void getData(const SevenZFileEntry &file, std::vector<unsigned char> &outBuffer);

    unsigned int nbFiles();

    using iterator = SevenZFileEntry *;
    using const_iterator = SevenZFileEntry const *;

    iterator begin() { return files.data(); }

    iterator end() { return files.data() + files.size(); }

    [[nodiscard]] const_iterator begin() const { return files.data(); }

    [[nodiscard]] const_iterator end() const { return files.data() + files.size(); }

private:
    struct MyCFileInStream;
    struct MyCLookToRead2;
    struct MyCSzArEx;

    std::unique_ptr<MyCFileInStream> archiveStream;
    std::unique_ptr<MyCLookToRead2> lookStream;
    std::unique_ptr<MyCSzArEx> db;

    std::vector<SevenZFileEntry> files;

    void check_error(int err) const;

    void check_errno(int err, const std::string &res) const;
};

#endif //INC_7Z_EXTRACTOR_SEVEN_Z_ARCHIVE_H
