#pragma once

#define UPLOAD_DIRECTORY "uploads/"

#include <string>
#include <sstream>
int stringToInt(const std::string& str);
std::string intToString(int value);
unsigned long stringToLong(const std::string& str);
std::string generateUniqueFilename();
double stringToDouble(const std::string& str);