#pragma once
#include <string>
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "server.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>
#include "utils.hpp"
#include <ctime>
#include <sys/time.h>


#include "ServerManager.hpp"
#include "Location.hpp"
#include "ConfigParser.hpp"
#include "Iconnect.hpp"

// below is timeout TIMES IN SEC.
#define HEADER_TIMEOUT 5
#define BODY_TIMEOUT 30
#define CONNECTION_TIMEOUT 3600