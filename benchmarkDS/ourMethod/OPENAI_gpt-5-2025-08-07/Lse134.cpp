#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <limits>

struct Route {
    std::string pattern;
    std::function<std::string(const std::string&)> func;
};

static std::vector<Route> g_routes;

static void register_route(const std::string& pattern, std::function<std::string(const std::string&)> func) {
    g_routes.push_back(Route{pattern, std::move(func)});
}

static std::string escape_html(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

static const size_t MAX_PATH_LEN = 2048;
static const size_t MAX_USERNAME_LEN = 256;

static std::string handle_request(const std::string& path) {
    if (path.empty() || path.size() > MAX_PATH_LEN) {
        return "400 Bad Request";
    }
    for (const auto& r : g_routes) {
        if (r.pattern == "/hello/{username}") {
            const std::string prefix = "/hello/";
            if (path.rfind(prefix, 0) != 0) { // not starting with prefix
                continue;
            }
            std::string username = path.substr(prefix.size());
            if (username.empty() || username.find('/') != std::string::npos) {
                continue;
            }
            if (username.size() > MAX_USERNAME_LEN) {
                return "400 Bad Request";
            }
            try {
                return r.func(username);
            } catch (...) {
                return "500 Internal Server Error";
            }
        }
    }
    return "404 Not Found";
}

static std::string hello(const std::string& username) {
    return std::string("Hello ") + escape_html(username);
}

// Simulated decorator through static registration macro
#define DECORATE_ROUTE(PATTERN, FUNC) \
    namespace { struct AutoReg_##FUNC { AutoReg_##FUNC(){ register_route(PATTERN, FUNC); } }; static AutoReg_##FUNC _areg_##FUNC; }

DECORATE_ROUTE("/hello/{username}", hello)

int main() {
    // 5 test cases
    std::vector<std::string> tests = {
        "/hello/Alice",
        "/hello/Bob<Admin>",
        "/hello/",
        "/hi/Alice",
        std::string("/hello/") + std::string(300, 'A')
    };
    for (const auto& t : tests) {
        std::cout << "URL: " << t << " -> " << handle_request(t) << "\n";
    }
    return 0;
}