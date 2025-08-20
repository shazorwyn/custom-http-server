#include "request.hpp"
#include <vector>
#include <sstream>
#include <algorithm>

std::vector<std::string> splitCommaSeparated(const std::string &value)
{
    std::vector<std::string> result;
    std::istringstream ss(value);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        result.push_back(token);
    }
    return result;
}

HttpRequest HttpRequest::parse(const std::string &raw)
{
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;

    // Read the request line
    std::getline(stream, line);
    std::istringstream lineStream(line);
    lineStream >> req.method >> req.path >> req.version;

    // Remove trailing \r from version if it exists
    while (std::getline(stream, line) && line != "\r")
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        auto colon = line.find(":");
        if (colon != std::string::npos)
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            value.erase(0, value.find_first_not_of(" "));
            req.headers[key] = value;
        }
    }

    req.body = std::string(std::istreambuf_iterator<char>(stream), {});
    return req;
}
