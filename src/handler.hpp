#pragma once

#include <string>
#include "request.hpp"
#include "response.hpp"

class HttpHandler
{
public:
    HttpHandler(const std::string &directory);

    HttpResponse handle(const HttpRequest &request);

private:
    std::string directory;

    HttpResponse handleGET(const HttpRequest &request);
    HttpResponse handlePOST(const HttpRequest &request);
};
