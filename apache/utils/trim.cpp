#include "../includes/utils.hpp"

void trim(std::string& str) {
    // Remove leading whitespace
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    
    // Remove trailing whitespace
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

std::string extractValue(const std::string& line, const std::string& directive) {
    size_t start = line.find(directive) + directive.length();
    std::string value = line.substr(start);
    
    // Remove leading spaces
    value.erase(0, value.find_first_not_of(" \t"));
    
    // Remove trailing semicolon if present
    if (!value.empty() && value[value.length() - 1] == ';') {
        value.pop_back();
    }
    
    trim(value);
    return value;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        trim(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}
