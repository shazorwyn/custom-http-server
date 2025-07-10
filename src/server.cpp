#include "server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <string>

HttpServer::HttpServer(int port) : port(port), server_fd(-1) {}

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

        // Print the request
        std::string request(buffer);
        std::cout << "Request:\n"
                  << request << std::endl;

        // Parse request line
        std::istringstream requestStream(request);
        std::string method, path, version;
        requestStream >> method >> path >> version;

        // Simple response handling
        std::string response;

        if (method == "GET")
        {
            if (path == "/")
            {
                response = "HTTP/1.1 200 OK\r\n\r\n";
            }
            else if (path.rfind("/echo/", 0) == 0)
            {                                         // path starts with /echo/
                std::string to_echo = path.substr(6); // everything after /echo/
                response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: " +
                    std::to_string(to_echo.size()) + "\r\n"
                                                     "\r\n" +
                    to_echo;
            }
            else if (path == "/user-agent")
            {
                std::string user_agent;
                std::istringstream req_lines(request);
                std::string line;
                while (std::getline(req_lines, line))
                {
                    if (!line.empty() && line.back() == '\r')
                        line.pop_back(); // Remove trailing \r
                    if (line.find("User-Agent:") == 0 || line.find("user-agent:") == 0)
                    {
                        // size_t colon_pos = line.find(":");
                        // if (colon_pos != std::string::npos)
                        user_agent = line.substr(line.find(":") + 2); // Extract User-Agent value
                        break;
                    }
                }
                response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: " +
                    std::to_string(user_agent.size()) + "\r\n"
                                                        "\r\n" +
                    user_agent;
            }
            else
            {
                response = "HTTP/1.1 404 Not Found\r\n\r\n";
            }
        }
        else
        {
            response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        }
        std::cout << "Response:\n"
                  << response << std::endl;

        send(client_socket, response.c_str(), response.size(), 0);
        close(client_socket);
    }

    close(server_fd);
}
