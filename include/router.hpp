#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include "request.hpp"
#include "response.hpp"

class Router
{
public:
    using HandlerFunc = std::function<HttpResponse(const HttpRequest &)>;

    void get(const std::string &path, HandlerFunc handler);
    void post(const std::string &path, HandlerFunc handler);
    HttpResponse route(const HttpRequest &request) const;

private:
    std::unordered_map<std::string, HandlerFunc> get_routes;
    std::unordered_map<std::string, HandlerFunc> post_routes;
};
