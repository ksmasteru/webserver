#include <iostream>

std::string getFolderName(const std::string& path)
{
    size_t pos = path.find('/', 0);
    if (pos == std::string::npos)
        return (""); // no path.
    return (path.substr(0, pos));
}

 int main(int ac, char **av)
 {
    if (ac != 2)
        return (0);
    std::string folderName = getFolderName(av[1]);
    std::cout << "folderName ist: " << folderName << std::endl;
 }