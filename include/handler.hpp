#pragma once

#include <string>
#include "request.hpp"
#include "response.hpp"
#include "router.hpp"

class HttpHandler
{
public:
    HttpHandler(const std::string &directory);

    HttpResponse handle(const HttpRequest &request);

private:
    std::string directory;
    Router router;

    void setupRoutes();

    HttpResponse rootHandler(const HttpRequest &request);
    HttpResponse echoHandler(const HttpRequest &request);
    HttpResponse userAgentHandler(const HttpRequest &request);
    HttpResponse fileGetHandler(const HttpRequest &request);
    HttpResponse filePostHandler(const HttpRequest &request);
};