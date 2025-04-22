#include "../includes/webserver.hpp"
#include "../includes/server.hpp"

int main()
{
    Server apache;
    try {
        apache.establishServer();
    }
    catch (const char *error_msg)
    {
        std::cout << "error :"
    }
}