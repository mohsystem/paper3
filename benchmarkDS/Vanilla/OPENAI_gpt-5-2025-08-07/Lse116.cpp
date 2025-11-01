#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

template <typename... Args>
std::string print(const char* fmt, Args... args) {
    int n = std::snprintf(nullptr, 0, fmt, args...);
    if (n < 0) return std::string();
    std::vector<char> buf(static_cast<size_t>(n) + 1);
    std::snprintf(buf.data(), buf.size(), fmt, args...);
    return std::string(buf.data(), static_cast<size_t>(n));
}

template <typename... Args>
int run_command(const char* fmt, Args... args) {
    std::string cmd = print(fmt, args...);
    return std::system(cmd.c_str());
}

int main() {
    run_command("ls");
    run_command("ls %s", "-l");
    run_command("ls %s", "-a");
    run_command("ls %s %s", "-l", ".");
    run_command("ls %s", "-1");
    return 0;
}