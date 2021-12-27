#include "Request.h"

Request::Request() {
    url = "";
    info = new RequestInfo();
}

Request::Request(std::string _url) {
    url = _url;
}

Request::Request(std::string _url, RequestInfo _info) {

}