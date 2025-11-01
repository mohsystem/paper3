#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <cstdio>
#include <vector>

struct PingResult {
    bool alive;
    std::string pong;
};

static std::map<std::string, std::function<PingResult(const std::string&)>> routes;

struct RouteRegistrar {
    RouteRegistrar(const std::string& pattern, std::function<PingResult(const std::string&)> fn) {
        routes[pattern] = std::move(fn);
    }
};

static std::string run_ping(const std::string& url, bool& aliveOut) {
#ifdef _WIN32
    std::string cmd = "ping -n 1 " + url;
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    std::string cmd = "ping -c 1 " + url;
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) {
        aliveOut = false;
        return "Failed to start ping";
    }
    std::string output;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
#ifdef _WIN32
    int rc = _pclose(pipe);
    int code = rc;
#else
    int status = pclose(pipe);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
    aliveOut = (code == 0);
    return output;
}

PingResult ping(const std::string& url) {
    bool alive = false;
    std::string pong = run_ping(url, alive);
    return PingResult{alive, pong};
}

static RouteRegistrar __route_ping("<url> ping", ping);

PingResult handle_request(const std::string& requestPath) {
    auto it = routes.find("<url> ping");
    if (it != routes.end()) {
        if (requestPath.size() >= 5 && requestPath.rfind(" ping") == requestPath.size() - 5) {
            std::string url = requestPath.substr(0, requestPath.size() - 5);
            // trim trailing spaces
            while (!url.empty() && (url.back() == ' ' || url.back() == '\t')) url.pop_back();
            return it->second(url);
        }
    }
    return PingResult{false, "No route matched"};
}

int main() {
    std::vector<std::string> tests = {
        "localhost ping",
        "127.0.0.1 ping",
        "google.com ping",
        "example.com ping",
        "no.such.domain.invalid ping"
    };
    for (const auto& t : tests) {
        PingResult r = handle_request(t);
        std::cout << t << " -> alive=" << (r.alive ? "true" : "false") << ", pong_len=" << r.pong.size() << "\n";
    }
    return 0;
}