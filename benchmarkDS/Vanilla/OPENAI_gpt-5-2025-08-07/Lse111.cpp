#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>

std::string ping(const std::string& inputUrl) {
    std::string url = inputUrl; // store input in variable called url
#ifdef _WIN32
    std::string cmd = "ping -n 1 -w 3000 \"" + url + "\" 2>&1";
#else
    std::string cmd = "ping -c 1 -W 3 \"" + url + "\" 2>&1";
#endif
    std::string output;
#if defined(_WIN32)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) {
        return "Failed to run ping command.";
    }
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

int main() {
    std::vector<std::string> tests = {
        "localhost",
        "127.0.0.1",
        "8.8.8.8",
        "example.com",
        "invalid.host"
    };
    for (const auto& t : tests) {
        std::cout << "=== Pinging: " << t << " ===" << std::endl;
        std::cout << ping(t) << std::endl;
    }
    return 0;
}