#pragma once

#include <string>

class HttpServer
{
public:
    HttpServer(int port, const std::string &directory = ".");
    void start();

private:
    int server_fd;
    int port;

    void setupSocket();
    std::string directory;
};
