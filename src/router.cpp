#include "router.hpp"

void Router::get(const std::string &path, HandlerFunc handler)
{
    get_routes[path] = handler;
}

void Router::post(const std::string &path, HandlerFunc handler)
{
    post_routes[path] = handler;
}

HttpResponse Router::route(const HttpRequest &request) const
{
    const auto &routes = request.method == "GET" ? get_routes : post_routes;

    auto it = routes.find(request.path);
    if (it != routes.end())
    {
        return it->second(request);
    }

    for (const auto &[routePath, handler] : routes)
    {
        if (routePath.back() == '/' && request.path.rfind(routePath, 0) == 0)
        {
            return handler(request);
        }
    }

    return HttpResponse(404, "Not Found");
}
