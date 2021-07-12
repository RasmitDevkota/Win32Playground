#include <map>

struct RequestInfo {
    std::map<std::string, RequestInfoFieldInterface> info;
};

class RequestInfoFieldInterface { };

template <typename T>
class RequestInfoField : public RequestInfoFieldInterface {
    T fieldValue;
};