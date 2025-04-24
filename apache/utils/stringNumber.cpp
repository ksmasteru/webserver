#include "../includes/utils.hpp"

int stringToInt(const std::string& str) {
    std::istringstream iss(str);
    int num;
    iss >> num;
    return num;
}

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

double stringToLongDouble(const std::string& str) {
    std::istringstream iss(str);
    double value;
    iss >> value;
    return value;
}