#include "../includes/utils.hpp"
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