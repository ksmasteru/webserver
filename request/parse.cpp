#include <cctype>
#include <iostream>
#include <stdio.h>
#include <string>
#include <map>

bool isGetRequest(char *str)
{
    return (str[0] == 'G' && str[1] == 'E' && str[2] == 'T'
        && str[3] == ' ');
}
bool isPostRequest(char *str)
{
    return (str[0] == 'P' && str[1] == 'O' && str[2] == 'S'
        && str[3] == 'T' && str[4] == ' ');
}

bool isDeleteRequest(char *str)
{
    return (str[0] == 'D' && str[1] == 'E' && str[2] == 'L'
        && str[3] == 'E' && str[4] == 'T' && str[5] == ' ');
}
bool isHttp(char *str)
{
    printf("word is %s\n", str); 
    return (str[0] == 'H' && str[1] == 'T' && str[2] == 'T'
        && str[3] == 'P' && str[4] == '/');
}

class Request{
    public:
        std::string targetUri;
        std::string type;
        char httpMinor;
        char httpGreater;
        // queries ?
        std::map <std::string, std::string> queries;
        std::string querykey;
        std::string queryvalue;
};

int main(int ac, char **av)
{
    if (ac != 2)
        return (0);
    // GET /where?q=now&blabla HTTP/1.1
    Request request;
    std::string qname;
    std::string qvalue;
    enum{
        start = 0,
        method_name,
        after_method_space,
        request_uri,
        question_mark,
        query_equal_mark,
        querykey,
        queryValue,
        spaceafterurl,
        HTTPword,
        httpgreat,
        dot,
        httpminor,
        CR,
        LF
    }state;
    enum
    {
        fieldname,
        OWS1,
        fieldvalue,
        OWS2,
        end
    }filed_state;
    char c,p ,*beg, *end;
    int first = 0;
    // check and at the same time parse values.
    state = start;
    for (int i = 0; i < strlen(av[1]); i++)
    {
        c = av[1][i];
        switch (state)
        {
            case start:
                first = i;
                if (c == '\r' || c == '\n') // skips this line ?
                    break;
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    return (printf("bad request start\n"));
                // else move on.
                state = method_name;
                break;
            case method_name: // check valid characters till space
                if (c == ' ')
                {
                    // calculate the diff between begin and end;
                    printf("i - 1first is %d\n", i - first);
                    int n = i - first;
                    printf("to compare is %s\n", av[1] + start);
                    switch (n)
                    {
                        case 3: // Get
                            if (isGetRequest(av[1] + first))
                                request.type = "GET";
                            break;
                        case 4: // Post 
                            if (isPostRequest(av[1] + first))
                                request.type = "POST";
                            break;
                        case 6: //  delete
                            if (isDeleteRequest(av[1] + first))
                                request.type = "DELETE";
                            break;
                        default:
                            return (printf("Bad Request method name\n"));
                    } 
                    if (request.type.empty())
                        return (printf("bad request name\n")); 
                    state = after_method_space;
                    break;
                }
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    return (printf("http parse invalid method"));
                break;
                case after_method_space: // this code focuses on origin form 
                    printf("after method space\n");
                    if (c == '/') // makes you jump host parsing
                    {
                        request.targetUri += c;
                        state = request_uri;
                        // also mark the start and end of each field
                        break;
                    }
                    return (printf("bad request no slash\n"));
                case request_uri: // /path?key1=val1&key2=val2&key3=val3
                    // request uri form first char to ? or space
                    printf("request uri c:%c\n",c);
                    switch (c)
                    {
                        case '?':
                            state = question_mark;
                            break;
                        case ' ':
                            state = spaceafterurl;
                            break;
                        default :
                            request.targetUri += c;
                            break;
                    }
                    break;
                case question_mark: // ?'fdsfds'=fdfsd
                    printf("question mark\n");
                    switch (c)
                    {
                        case '=':
                            state = query_equal_mark;
                            break;
                        case ' ':
                            return printf("bad request question mark no value\n");
                        case '?':
                            return printf("bad request multiple '?'\n");
                        case '&':
                            return printf("bad request no key before &\n");
                        default:
                            request.querykey += c;
                            break;
                    }
                    break;
                case query_equal_mark:
                    printf("q equal mark c:%c\n", c);
                    if (request.querykey.empty())
                        return (printf("bad request no query key\n"));
                    if (c < 0x20 || c == 0x7f || c == '?') /*no printib chars*/
                        return (printf("bad request uri\n"));
                    if(c == ' ' || c == '&')
                    {
                        if (request.queryvalue.empty())
                            return (printf("bad reqeust query done\n"));
                        request.queries[request.querykey] = request.queryvalue;
                        request.querykey.clear();
                        request.queryvalue.clear();
                        if (c == ' ')
                            state = spaceafterurl;
                        else
                            state = querykey;
                        break;
                    }
                    else
                        request.queryvalue += c;
                    break;
                case querykey:
                    if (c == '&' || c == ' ' || c == '?')
                        return (printf("bad request\n"));
                    if (c == '=')
                    {
                        state = queryValue;
                        break;
                    }
                    request.querykey += c;
                    break;
                case queryValue:
                    if (request.querykey.empty())
                        return printf("bad request : no query key\n");
                    if (c == '?')
                        return (printf("bad request multiple '?'\n"));
                    if (c == '&' || c == ' ')
                    {
                        if (request.queryvalue.empty())
                            return (printf("bad reqeust query done\n"));
                        request.queries[request.querykey] = request.queryvalue;
                        request.querykey.clear();
                        request.queryvalue.clear();
                        if (c == '&')
                            state = querykey;
                        else
                            state = queryValue;
                        break;
                    }
                    request.queryvalue += c;
                    break;
                case spaceafterurl:
                    if (c != 'H')
                        return printf(("bad request\n"));
                    first = i;
                    state = HTTPword;
                    break;
                case HTTPword:
                    switch (c)
                    {
                        case '/':
                        {
                            int n = i - first;
                            printf("first is %d n is %d\n", first, n);
                            switch(n)
                            {
                                case 4:
                                    if (!isHttp(av[1] + first))
                                        return printf(("bad http word1\n"));
                                    state = httpgreat;
                                    break;
                                default:
                                    return (printf("bad http word2\n"));
                            }
                        }
                        default:
                            break;
                    }
                break;
                case httpgreat:
                    if (c != '1' && c != '0')
                        return (printf("bad http version\n"));
                    request.httpGreater = c;                
                    state = dot;
                    break;
                case dot:
                    if (c != '.')
                        return (printf("no dot in http format\n"));
                    state = httpminor;
                    break;
                case httpminor:
                    if (c != '1' && c != '0')
                        return printf("bad http minor format\n");
                    request.httpMinor = c;
                    state = CR;
                    break;
                case CR: // demo parsing 
                    if (c != '\r')
                        return printf("bad request\n");
                break;
                case LF:
                    if (c != '\n')
                        throw ("bad request line no new line\n");
            }
        }
        // printing keys and values of queries.
        std::map<std::string, std::string>::iterator it;
        for (it =  request.queries.begin(); it != request.queries.end(); ++it)
        {
            std::cout << "query key " << it->first << " query value " << it->second << std::endl;
        }
        printf("state is %d", state);
}
