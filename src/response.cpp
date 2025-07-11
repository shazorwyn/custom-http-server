#include "response.hpp"
#include <sstream>

HttpResponse::HttpResponse(int code, std::string message)
    : status_code(code), status_message(std::move(message)) {}

void HttpResponse::setHeader(const std::string &key, const std::string &value)
{
    headers[key] = value;
}

std::string HttpResponse::toString() const
{
    std::ostringstream res;

    res << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
    for (const auto &[key, value] : headers)
    {
        res << key << ": " << value << "\r\n";
    }
    res << "\r\n"
        << body;

    return res.str();
}
