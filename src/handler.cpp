#include "handler.hpp"
#include <fstream>
#include <sstream>

HttpHandler::HttpHandler(const std::string &directory) : directory(directory) {}

HttpResponse HttpHandler::handle(const HttpRequest &request)
{
    if (request.method == "GET")
    {
        return handleGET(request);
    }
    else if (request.method == "POST")
    {
        return handlePOST(request);
    }
    else
    {
        return HttpResponse(405, "Method Not Allowed");
    }
}

HttpResponse HttpHandler::handleGET(const HttpRequest &request)
{
    if (request.path == "/")
    {
        return HttpResponse(200, "OK");
    }

    if (request.path.rfind("/echo/", 0) == 0)
    {
        std::string msg = request.path.substr(6);
        HttpResponse res(200, "OK");
        res.body = msg;
        res.setHeader("Content-Type", "text/plain");
        res.setHeader("Content-Length", std::to_string(msg.size()));
        return res;
    }

    if (request.path == "/user-agent")
    {
        auto it = request.headers.find("User-Agent");
        std::string agent = (it != request.headers.end()) ? it->second : "";

        HttpResponse res(200, "OK");
        res.body = agent;
        res.setHeader("Content-Type", "text/plain");
        res.setHeader("Content-Length", std::to_string(agent.size()));
        return res;
    }

    if (request.path.rfind("/files/", 0) == 0)
    {
        std::string filename = request.path.substr(7);
        if (filename.find("..") != std::string::npos)
        {
            return HttpResponse(403, "Forbidden");
        }

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

    return HttpResponse(404, "Not Found");
}

HttpResponse HttpHandler::handlePOST(const HttpRequest &request)
{
    if (request.path.rfind("/files/", 0) != 0)
    {
        return HttpResponse(404, "Not Found");
    }

    std::string filename = request.path.substr(7);
    if (filename.find("..") != std::string::npos)
    {
        return HttpResponse(403, "Forbidden");
    }

    std::ofstream out(directory + "/" + filename, std::ios::binary);
    out << request.body;

    return HttpResponse(201, "Created");
}
