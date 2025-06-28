#include <iostream>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <sstream>
#include <vector>

std::string formatDate(time_t rawtime) {
    char buffer[20];
    std::tm* timeinfo = std::localtime(&rawtime);
    std::strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M", timeinfo);
    return std::string(buffer);
}

std::string humanSize(off_t size) {
    std::ostringstream oss;
    if (size >= (1 << 20))
        oss << std::fixed << std::setprecision(1) << (size / (1024.0 * 1024)) << "M";
    else if (size >= (1 << 10))
        oss << (size / 1024) << "K";
    else
        oss << size << "B";
    return oss.str();
}

int main() {
    const char* path = "./pages";
    DIR* dir = opendir(path);
    std::vector<std::string> listing;
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
            continue;

        struct stat st;
        std::string fullpath = std::string(path) + "/" + entry->d_name;
        if (stat(fullpath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            std::ostringstream line;
            
            line << std::setw(20) << formatDate(st.st_mtime)
                 << std::right << std::setw(6) << humanSize(st.st_size);

            listing.push_back(line.str());
        }
    }

    closedir(dir);

    // Example: print the vector content to verify
    for (const auto& line : listing) {
        std::cout << line << '\n';
    }

    return 0;
}
