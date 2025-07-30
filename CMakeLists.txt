cmake_minimum_required(VERSION 3.10)
project(http_server)

set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include)

# Add sources
add_executable(http_server
    main.cpp
    src/server.cpp
    src/request.cpp
    src/response.cpp
    src/handler.cpp
    src/router.cpp
)
