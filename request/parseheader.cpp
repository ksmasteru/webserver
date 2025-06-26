#include <cctype>
#include <iostream>
#include <stdio.h>
#include <string>
#include <map>

enum
{
        name,
        OWS1,
        val,
        done
    }fieldstate;

int main(int ac, char **av)
{
    //offset and total read
    enum
    {
        name,
        OWS1,
        val,
        OWS2,
        CR,
        LF,
    }fieldstate;
    char c;
    fieldstate = name;
    std::string fieldname, fieldvalue;
    //parseRequestLine();
    int offset = 0;
    int _bytesread = strlen(av[1]);
    char *request = av[1];
    std::map<std::string, std::string> headers;
    try {
    for (; offset < _bytesread; offset++)
    {
        c = request[offset];
        switch (fieldstate)
        {
            case name://some chars arent allowed.
                //printf("Case method\n");
                if (isspace(c))
                    throw("bad request field name\n");
                if (c == ':')
                {
                    if (fieldname.empty())
                        throw ("empty field name\n"); 
                    fieldstate = OWS1;
                }
                else
                    fieldname += c;
                break;
            case OWS1:
                fieldstate = val;
                if (isspace(c))
                    break;
            case val:
                //printf("val\n");
                if (isspace(c) || c == 'r')
                {
                    if (fieldvalue.empty())
                        throw("emptyvalue\n");
                    fieldstate = OWS2;
                }
                else
                {
                    fieldvalue += c;
                    break;
                }
            case OWS2:
                fieldstate = CR;
                if (isspace(c))
                    break;
            case CR:
                if (c != 'r')
                    throw ("no r line\n");
                fieldstate = LF;
                break;
            case LF:
                if (c != 'n')
                    throw ("no new line\n");
                fieldstate = name;
                headers[fieldname] = fieldvalue;
                fieldvalue.clear();
                fieldname.clear();
                break;
        }
    }
        if (fieldstate != 0 || !fieldname.empty())
            throw ("bad request field\n");
    }
    catch (const char *error)
    {
        std::cout << error << std::endl;
    }
    printf("switch state is %d\n", fieldstate);
    for (auto it = headers.begin(); it != headers.end(); ++it)
        std::cout << "key is " << it->first << " value is " << it->second << std::endl;

}