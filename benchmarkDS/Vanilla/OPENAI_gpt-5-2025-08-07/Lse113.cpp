#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

struct PingResponse {
    bool alive;
    std::string pong;
};

PingResponse ping(const std::string& url) {
#ifdef _WIN32
    std::string cmd = "ping -n 1 " + url;
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    std::string cmd = "ping -c 1 " + url;
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    std::string pong;
    if (!pipe) {
        return {false, "Error opening pipe for ping command"};
    }
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        pong += buffer;
    }
#ifdef _WIN32
    int status = _pclose(pipe);
    bool alive = (status == 0);
#else
    int status = pclose(pipe);
    bool alive = false;
    if (WIFEXITED(status)) {
        alive = (WEXITSTATUS(status) == 0);
    }
#endif
    return {alive, pong};
}

int main() {
    std::vector<std::string> tests = {
        "localhost",
        "127.0.0.1",
        "example.com",
        "google.com",
        "invalid.invalid"
    };
    for (const auto& t : tests) {
        PingResponse r = ping(t);
        std::cout << "URL: " << t << "\n";
        std::cout << "Alive: " << (r.alive ? "true" : "false") << "\n";
        std::cout << "Pong output length: " << r.pong.size() << "\n";
        std::cout << "----\n";
    }
    return 0;
}