#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>

std::string execCommand(const std::string& cmd) {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return result;
    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        result.append(buffer, n);
    }
    pclose(pipe);
    return result;
}

std::string getUserInfo(const std::string& username) {
    return execCommand("cat /etc/passwd | grep " + username);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        std::cout << getUserInfo(argv[1]);
    } else {
        std::vector<std::string> tests = {"root", "daemon", "bin", "sys", "nobody"};
        for (const auto& u : tests) {
            std::cout << getUserInfo(u);
        }
    }
    return 0;
}