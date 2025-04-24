#pragma once

#include "AResponse.hpp"

class  DelResponse : AResponse{
    public:
        DelResponse(std::string type, Request& res) : AResponse(type, res){}
        ~DelResponse();
};