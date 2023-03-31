#pragma once
#include <vector>

//adds null terminator
void loadWholeTextFile(const char* fname, std::vector<char> &data);
void loadWholeBinFile(const char* fname, std::vector<char>& data);
void makeEmptyFile(const char* fname);