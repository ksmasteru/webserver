#include "../includes/utils.hpp"
#include <iostream>

int stringToInt(const std::string& str) {
    std::istringstream iss(str);
    int num;
    iss >> num;
    return num;
}

unsigned long stringToLong(const std::string& str)
{
    std::istringstream iss(str);
    unsigned long num;
    iss >> num;
    return (num); 
}

std::string longlongToString(long long value)
{
    std::ostringstream oss;
    oss << value;
    return (oss.str());
}

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

double stringToDouble(const std::string& str) {
    std::istringstream iss(str);
    double value;
    iss >> value;
    return value;
}


std::string generateUniqueFilename() {
    // Get current timestamp
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    // Format timestamp: YYYYMMDD_HHMMSS
    strftime(buffer, 80, "%Y%m%d_%H%M%S", timeinfo);
    std::string timestamp(buffer);
    
    // Generate random string (6 characters)
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomStr;
    
    // Initialize random seed
    srand(static_cast<unsigned int>(time(NULL) ^ clock()));
    
    for (int i = 0; i < 6; ++i) {
        int index = rand() % chars.size();
        randomStr += chars[index];
    }
    
    // Build filename: upload_YYYYMMDD_HHMMSS_[random]
    std::string filename = std::string(UPLOAD_DIRECTORY) + "upload_" + timestamp + "_" + randomStr;
    
    // Add appropriate extension based on Content-Type if provided
    return filename;
}

unsigned long hexStringToLong(const std::string& hexStr) {
    unsigned long result = 0;
    std::size_t i = 0;

    // Skip optional "0x" or "0X"
    if (hexStr.size() >= 2 && hexStr[0] == '0' &&
        (hexStr[1] == 'x' || hexStr[1] == 'X')) {
        i = 2;
    }

    for (; i < hexStr.size(); ++i) {
        char c = hexStr[i];
        int digit;

        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (std::tolower(c) >= 'a' && std::tolower(c) <= 'f')
            digit = 10 + (std::tolower(c) - 'a');
        else
            break;
        result = result * 16 + digit;
    }
    std::cout << "for " << hexStr << " conversion is " << result << std::endl;
    return result;
}
