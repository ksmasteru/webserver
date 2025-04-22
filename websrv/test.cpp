/* testing stringstream
* 
*
*/
#include <iostream> 
#include <sstream>
#include <string>
#include <fstream>
#include <ctime>
#include <cstring>


int main() {
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);  // Get GMT time

    char buffer[100];
    // Format: "Mon, 22 Apr 2025 10:00:00 GMT"
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    std::cout << buffer << std::endl;

    return 0;
}
