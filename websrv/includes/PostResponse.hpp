#pragma once

#include "AResponse.hpp"

class  PostResponse : AResponse{
    public:
        PostResponse(std::string type, Request& res) : AResponse(type, res){}
        ~PostResponse();
};