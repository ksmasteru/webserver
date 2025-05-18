#pragma once

#define UPLOAD_DIRECTORY "uploads/"

#include <string>
#include <sstream>
int stringToInt(const std::string& str);
std::string intToString(int value);
double stringToLongDouble(const std::string& str);
std::string generateUniqueFilename();