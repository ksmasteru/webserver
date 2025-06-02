#pragma once

#define UPLOAD_DIRECTORY "uploads/"

#include <string>
#include <sstream>
#include <vector>

int stringToInt(const std::string& str);
std::string intToString(int value);
unsigned long stringToLong(const std::string& str);
std::string generateUniqueFilename();
double stringToDouble(const std::string& str);
unsigned long hexStringToLong(const std::string& hexStr);

// trim.cpp
void trim(std::string& str);
std::string extractValue(const std::string& line, const std::string& directive);
std::vector<std::string> split(const std::string& str, char delimiter);