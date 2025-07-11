#pragma once
#include <string>
#include <unordered_map>

class HttpResponse
{
public:
    int status_code;
    std::string status_message;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    HttpResponse(int code = 200, std::string message = "OK");

    void setHeader(const std::string &key, const std::string &value);
    std::string toString() const;
};
