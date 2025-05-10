#pragma once

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

    public:
        Response1(){
            state = start;
        }
        ~Response1();
        int getState(){return state;}
};