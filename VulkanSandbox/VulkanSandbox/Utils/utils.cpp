#include "utils.h"
#include <filesystem>
#include <iostream>
#include "CLogger.h"

using namespace std;
using namespace std::filesystem;

void loadWholeBinFile(const char* fname, vector<char>& data)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fname, "rb");

    Assert(file, "Failed to load file", { {"file", fname},  {"error code", err} });

    size_t fsize = file_size(fname);

    data.resize(fsize);
    fread_s(data.data(), fsize, 1, fsize, file);
    fclose(file);
}

void loadWholeTextFile(const char* fname, vector<char>& data)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fname, "rb");

    Assert(file, "Failed to load file", { {"file", fname},  {"error code", err} });

    size_t fsize = file_size(fname);

    data.resize(fsize + 1);
    fread_s(data.data(), fsize + 1, 1, fsize, file);

    fclose(file);

    data[fsize] = 0;
}

void makeEmptyFile(const char* fname)
{
    path folder(path(fname).parent_path());

    if (!exists(folder))
    {
        create_directories(folder);
    }

    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fname, "wb");

    if (!file)
    {
        perror("fopen");
    }

    Assert(file, "Failed to load file", { {"file", fname},  {"error code", err} });

    fclose(file);
}
