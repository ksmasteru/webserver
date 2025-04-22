#pragma once

#include "AResponse.hpp"

class GetResponse : public AResponse
{
    public:
        GetResponse(std::string type, Request& req) : AResponse(type, req){}
        ~GetResponse();
};