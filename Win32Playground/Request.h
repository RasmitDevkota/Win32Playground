#include "RequestInfo.h"
#include <string>

struct Request {
    std::string url;
    RequestInfo* info;
    
    Request();

    Request(std::string _url);

    Request(std::string _url, RequestInfo _info);


    class Request
    {
    };
};