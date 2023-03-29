#include "utils.h"
#include <filesystem>
#include <iostream>
#include "CLogger.h"

using namespace std;

char* loadWholeBinFile(const char* fname)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fname, "rb");

	Assert(file, "Failed to load file", { {"file", fname},  {"error code", err} });

    size_t fsize = filesystem::file_size(fname);

    char* fileData = new char[fsize];
    fread_s(fileData, fsize, 1, fsize, file);
    fclose(file);

    return fileData;
}

char* loadWholeTextFile(const char* fname)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fname, "rb");
    
    Assert(file, "Failed to load file", { {"file", fname},  {"error code", err} });
    
    size_t fsize = filesystem::file_size(fname);

    char* fileData = new char[fsize + 1];
    fread_s(fileData, fsize + 1, 1, fsize, file);

    fclose(file);

    fileData[fsize] = 0;

    return fileData;
}

void deleteFileData(char* data)
{
    delete[] data;
}

void makeEmptyFile(const char* fname)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fname, "wb");
    
    Assert(file, "Failed to load file", { {"file", fname},  {"error code", err} });

    fclose(file);
}
