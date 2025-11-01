#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/wait.h>
#define POPEN popen
#define PCLOSE pclose
#endif

struct PingResult {
    std::string url;
    std::string pong;
    bool alive;
    int exit_code;
};

using RouteHandler = std::function<PingResult(const std::string&)>;

static std::unordered_map<std::string, RouteHandler>& routes_map() {
    static std::unordered_map<std::string, RouteHandler> routes;
    return routes;
}

#define ROUTE(path, func) \
    static bool _route_reg_##func = [](){ routes_map().emplace(path, func); return true; }();

static const size_t MAX_OUTPUT = 8192;

static std::string extract_and_validate_host(const std::string& url) {
    std::string s = url;
    // trim
    auto ltrim = [](const std::string& str)->std::string{
        size_t i = 0; while (i < str.size() && isspace(static_cast<unsigned char>(str[i]))) ++i; return str.substr(i);
    };
    auto rtrim = [](const std::string& str)->std::string{
        size_t i = str.size(); while (i > 0 && isspace(static_cast<unsigned char>(str[i-1]))) --i; return str.substr(0, i);
    };
    s = rtrim(ltrim(s));
    if (s.empty() || s.size() > 2048) throw std::runtime_error("invalid url");

    std::string host = s;
    auto pos = s.find("://");
    if (pos != std::string::npos) {
        size_t start = pos + 3;
        size_t end = s.find('/', start);
        host = (end == std::string::npos) ? s.substr(start) : s.substr(start, end - start);
        // strip userinfo if present
        size_t at = host.rfind('@');
        if (at != std::string::npos) host = host.substr(at + 1);
        // drop port
        // for IPv6 address with brackets, we'll strip them later
        if (!host.empty() && host[0] != '[') {
            size_t colon = host.find(':');
            if (colon != std::string::npos) host = host.substr(0, colon);
        }
    }
    // remove brackets for IPv6
    if (!host.empty() && host.front() == '[' && host.back() == ']') {
        host = host.substr(1, host.size() - 2);
    }
    if (host.empty() || host.size() > 253) throw std::runtime_error("invalid host length");

    // validate allowed characters only
    for (char ch : host) {
        bool ok = (ch >= 'a' && ch <= 'z') ||
                  (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') ||
                  ch == '.' || ch == '-' || ch == ':'; // IPv6 colon
        if (!ok) throw std::runtime_error("invalid host characters");
    }
    if (host.find(' ') != std::string::npos || host.find(';') != std::string::npos ||
        host.find('|') != std::string::npos || host.find('&') != std::string::npos ||
        host.find('$') != std::string::npos || host.find('`') != std::string::npos ||
        host.find('\\') != std::string::npos) {
        throw std::runtime_error("invalid host characters");
    }
    return host;
}

PingResult ping(const std::string& url) {
    std::string pong;
    int exit_code = -1;
    bool alive = false;
    std::string host;
    try {
        host = extract_and_validate_host(url);
    } catch (...) {
        return {url, "invalid url/host", false, -1};
    }

#if defined(_WIN32)
    std::string cmd = "ping -n 1 -w 2000 " + host;
#else
    std::string cmd = "ping -c 1 -W 2 " + host;
#endif

    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe) {
        return {url, "failed to run ping", false, -1};
    }

    char buffer[256];
    pong.reserve(512);
    size_t total = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        size_t len = strnlen(buffer, sizeof(buffer));
        if (total + len > MAX_OUTPUT) {
            size_t copy = (MAX_OUTPUT > total) ? (MAX_OUTPUT - total) : 0;
            pong.append(buffer, buffer + copy);
            total += copy;
            break;
        } else {
            pong.append(buffer, buffer + len);
            total += len;
        }
    }
    int status = PCLOSE(pipe);
#if defined(_WIN32)
    exit_code = status;
#else
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else {
        exit_code = -1;
    }
#endif
    alive = (exit_code == 0);
    return {url, pong, alive, exit_code};
}

PingResult ping_handler(const std::string& url) {
    return ping(url);
}

ROUTE("/ping", ping_handler)

PingResult handle_request(const std::string& path, const std::string& url) {
    auto it = routes_map().find(path);
    if (it == routes_map().end()) {
        return {url, "route not found", false, -1};
    }
    try {
        return it->second(url);
    } catch (...) {
        return {url, "handler error", false, -1};
    }
}

int main() {
    std::vector<std::string> tests = {
        "127.0.0.1",
        "localhost",
        "https://example.com",
        "invalid host!!",
        "8.8.8.8"
    };
    for (const auto& t : tests) {
        PingResult r = handle_request("/ping", t);
        std::cout << "url=" << r.url << ", alive=" << (r.alive ? "true" : "false") << ", exit_code=" << r.exit_code
                  << ", pong_len=" << r.pong.size() << "\n";
        std::string sample = r.pong.size() > 120 ? r.pong.substr(0, 120) + "..." : r.pong;
        std::cout << "Sample of pong: " << sample << "\n-----\n";
    }
    return 0;
}