#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

class Router {
public:
    static Router& instance() {
        static Router inst;
        return inst;
    }

    void registerRoute(const std::string& path, std::function<std::string(const std::string&)> handler) {
        routes[path] = handler;
    }

    std::string handleRequest(const std::string& url) {
        const std::string prefix = "/hello/";
        if (url.rfind(prefix, 0) == 0) {
            auto it = routes.find("/hello");
            if (it != routes.end()) {
                std::string username = url.substr(prefix.size());
                return it->second(username);
            }
        }
        return "404 Not Found";
    }

private:
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> routes;
};

std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default: out += c; break;
        }
    }
    return out;
}

#define ROUTE(PATH, NAME) \
    std::string NAME(const std::string& username); \
    struct NAME##_registrar { NAME##_registrar(){ Router::instance().registerRoute(PATH, NAME); } }; \
    static NAME##_registrar NAME##_registrar_instance; \
    std::string NAME(const std::string& username)

ROUTE("/hello", hello) {
    return std::string("Hello ") + escape(username);
}

int main() {
    std::string tests[] = {
        "/hello/world",
        "/hello/Alice&Bob",
        "/hello/Eve<Admin>",
        "/hello/O'Reilly",
        "/hello/Tom \"The Tiger\""
    };
    for (const auto& t : tests) {
        std::cout << Router::instance().handleRequest(t) << std::endl;
    }
    return 0;
}