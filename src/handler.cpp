#include <fstream>
#include <sstream>

#include "handler.hpp"

HttpHandler::HttpHandler(const std::string &directory) : directory(directory)
{
    setupRoutes();
}

void HttpHandler::setupRoutes()
{
    // router.get("/", [this](const HttpRequest &request)
    //            { return rootHandler(request); });
    router.get("/user-agent", [this](const HttpRequest &request)
               { return userAgentHandler(request); });
    router.get("/files/", [this](const HttpRequest &request)
               { return fileGetHandler(request); });
    router.post("/files/", [this](const HttpRequest &request)
                { return filePostHandler(request); });
    router.get("/echo/", [this](const HttpRequest &request)
               { return echoHandler(request); });
}

HttpResponse HttpHandler::handle(const HttpRequest &request)
{
    return router.route(request);
}

// HttpResponse HttpHandler::rootHandler(const HttpRequest &request)
// {
//     return HttpResponse(200, "OK");
// }

HttpResponse HttpHandler::userAgentHandler(const HttpRequest &request)
{
    auto it = request.headers.find("User-Agent");
    std::string agent = (it != request.headers.end()) ? it->second : "";

    HttpResponse res(200, "OK");
    res.body = agent;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Length", std::to_string(agent.size()));
    return res;
}

HttpResponse HttpHandler::fileGetHandler(const HttpRequest &request)
{
    std::string filename = request.path.substr(7);
    if (filename.find("..") != std::string::npos)
        return HttpResponse(403, "Forbidden");

    std::ifstream file(directory + "/" + filename, std::ios::binary);
    if (!file)
        return HttpResponse(404, "Not Found");

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    HttpResponse res(200, "OK");
    res.body = content;
    res.setHeader("Content-Type", "application/octet-stream");
    res.setHeader("Content-Length", std::to_string(content.size()));
    return res;
}

HttpResponse HttpHandler::filePostHandler(const HttpRequest &request)
{
    std::string filename = request.path.substr(7);
    if (filename.find("..") != std::string::npos)
        return HttpResponse(403, "Forbidden");

    std::ofstream out(directory + "/" + filename, std::ios::binary);
    out << request.body;

    return HttpResponse(201, "Created");
}

HttpResponse HttpHandler::echoHandler(const HttpRequest &request)
{
    std::string msg = request.path.substr(6);
    HttpResponse res(200, "OK");
    res.body = msg;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Length", std::to_string(msg.size()));
    return res;
}
