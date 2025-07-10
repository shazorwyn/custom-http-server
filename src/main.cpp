#include "server.hpp"

#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    std::string directory = ".";

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--directory" && i + 1 < argc)
        {
            directory = argv[++i];
            break;
        }
    }

    HttpServer server(8080, directory);
    server.start();
    return 0;
}
