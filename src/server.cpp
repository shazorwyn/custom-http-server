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
void handleClient(int client_socket, const std::string &request, const std::string &directory)
{
    // Parse request line
    // std::istringstream requestStream(request);
    // std::string method, path, version;
    // requestStream >> method >> path >> version;

    std::string header_part, body_part;
    size_t header_end = request.find("\r\n\r\n");
    if (header_end != std::string::npos)
    {
        header_part = request.substr(0, header_end);
        body_part = request.substr(header_end + 4); // body starts after \r\n\r\n
    }
    std::istringstream header_stream(header_part);
    std::string method, path, version;
    header_stream >> method >> path >> version;

    std::string line;
    int content_length = 0;

    while (std::getline(header_stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.find("Content-Length:") == 0 || line.find("content-length:") == 0)
        {
            content_length = std::stoi(line.substr(line.find(":") + 1));
        }
    }

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
        else if (path.rfind("/files/", 0) == 0)
        {
            std::string file_path = path.substr(7); // gets everything after /files/
            if (file_path.find("..") != std::string::npos)
            {
                response = "HTTP/1.1 403 Forbidden\r\n\r\n";
                send(client_socket, response.c_str(), response.size(), 0);
                close(client_socket);
                return;
            } // Prevent directory traversal attacks

            std::string full_path = directory + "/" + file_path;
            std::ifstream file(full_path, std::ios::binary);

            if (file)
            {
                std::ostringstream content_stream;
                content_stream << file.rdbuf(); // read entire file
                std::string file_content = content_stream.str();

                response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/octet-stream\r\n"
                    "Content-Length: " +
                    std::to_string(file_content.size()) + "\r\n"
                                                          "\r\n" +
                    file_content;
            }
            else
            {
                response = "HTTP/1.1 404 Not Found\r\n\r\n";
            }
        }
        else
        {
            response = "HTTP/1.1 404 Not Found\r\n\r\n";
        }
    }
    else if (method == "POST" && path.rfind("/files/", 0) == 0)
    {
        std::string filename = path.substr(7);
        if (filename.find("..") != std::string::npos)
        {
            std::string response = "HTTP/1.1 403 Forbidden\r\n\r\n";
            send(client_socket, response.c_str(), response.size(), 0);
            close(client_socket);
            return;
        }

        std::string full_path = directory + "/" + filename;

        // Read body (we may have only part of it so far)
        while ((int)body_part.size() < content_length)
        {
            char extra_buf[1024] = {0};
            ssize_t bytes = read(client_socket, extra_buf, sizeof(extra_buf));
            if (bytes <= 0)
                break;
            body_part.append(extra_buf, bytes);
        }

        // Write to file
        std::ofstream outfile(full_path, std::ios::binary);
        outfile.write(body_part.c_str(), content_length);
        outfile.close();

        response = "HTTP/1.1 201 Created\r\n\r\n";
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
