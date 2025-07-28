#include "../inlcudes/Connection.hpp"

struct timeval Connection::getTime()
{
    return this->startTime;
}
struct timeval Connection::getConnectionTime()
{
    return (this->connectionTime);
}
void    Connection::resetTime()
{
    gettimeofday(&startTime, 0);
}

Connection::Connection(int client_fd, struct timeval& timeout)
{
    _timeOut = false;
    _client_fd = client_fd;
    startTime = timeout;
    _writeMode = false;
    gettimeofday(&connectionTime, nullptr);
}
// why is this empty
void Connection::resetConnection()
{
    
}

Connection::~Connection()
{

}