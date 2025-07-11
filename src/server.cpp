#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <string>
#include <thread>
#include <fstream>

#include "server.hpp"
#include "handler.cpp"
#include "request.cpp"
#include "response.cpp"

HttpServer::HttpServer(int port, const std::string &directory) : port(port), server_fd(-1), directory(directory) {}

void HttpServer::setupSocket()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "✅ Listening on port " << port << std::endl;
}
void handleClient(int client_socket, const std::string &raw_request, const std::string &directory)
{
    HttpRequest req = HttpRequest::parse(raw_request);

    auto it = req.headers.find("Content-Length");
    if (it != req.headers.end())
    {
        int expected_length = std::stoi(it->second);
        while ((int)req.body.size() < expected_length)
        {
            char buffer[1024];
            ssize_t bytes = read(client_socket, buffer, sizeof(buffer));
            if (bytes <= 0)
                break;
            req.body.append(buffer, bytes);
        }
    }

    HttpHandler handler(directory);
    HttpResponse res = handler.handle(req);

    std::string response_str = res.toString();
    send(client_socket, response_str.c_str(), response_str.size(), 0);
    close(client_socket);
}
void HttpServer::start()
{
    setupSocket();

    while (true)
    {
        // Accept a new connection
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        // Wait for a client to connect
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        // Handle the client request
        char buffer[1024] = {0};
        ssize_t bytesRead = read(client_socket, buffer, sizeof(buffer));
        if (bytesRead <= 0)
        {
            close(client_socket);
            continue;
        }

        std::string request(buffer);

        // Print the request
        // std::cout << "Request:\n"
        //           << request << std::endl;

        // handle each connection in a new thread

        std::thread(handleClient, client_socket, request, directory).detach(); // Detach the thread to allow it to run independently
    }

    close(server_fd);
}
