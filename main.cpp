#include "SevenZ.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
    fs::path input_file("/home/gabriel/IGN Files/SCAN25_3-1_TOUR_TIFF_LAMB93_D014_2022-12-01.7z");
//    fs::path input_file("../lzma2301.7z");
    fs::path out_dir("./test/");

    if (exists(out_dir)) {
        remove_all(out_dir);
    }

    create_directory(out_dir);

    SevenZArchive sevenZArchive = SevenZArchive(input_file);
    std::cout << "Nb Files : " << sevenZArchive.nbFiles() << std::endl;


    std::for_each(sevenZArchive.begin(), sevenZArchive.end(),
                  [&sevenZArchive, &out_dir](const SevenZFileEntry &fileEntry) {
                      if (fileEntry.fileName.extension().string() == ".tif") {
                          std::tm *ltm = std::localtime(&fileEntry.creationTime);
                          std::cout << "Name : " << fileEntry.fileName << std::endl
                                    << "Size : " << fileEntry.fileSize << std::endl
                                    << "Time : " << std::put_time(ltm, "%c %Z") << std::endl << std::endl;

                          sevenZArchive.extractInFile(fileEntry, out_dir / fileEntry.fileName.filename());
                      }
                  });

    return 0;
}