#pragma once
#include <string>

enum{
    start,
    sendingHeader,
    sendingBody,
    done
}Response_state;

class Response1{
    private:
        char *buffer;
        int state;
        std::string _type;
        int offset; // how much was sent;
        int totalbits;// total size of the ressource to send.
    public:
        Response1(){
            state = start;
        }
        ~Response1();
        int getState(){return state;}
        std::string getType(){
            return (this->_type);
        }
        char *getBuffer(){return this->buffer;}
};