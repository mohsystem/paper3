#include <cstdio>
#include <cstdarg>
#include <string>
#include <iostream>

std::string print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int needed = std::vsnprintf(nullptr, 0, fmt, args);
    va_end(args);
    if (needed < 0) return std::string();

    std::string out(needed, '\0');
    va_start(args, fmt);
    std::vsnprintf(out.data(), out.size() + 1, fmt, args);
    va_end(args);
    return out;
}

std::string execCommand(const std::string& cmd) {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

int main() {
    std::string cmds[5] = {
        print("ls %s", "-1"),
        print("ls"),
        print("ls %s %s", "-a", "-1"),
        print("ls %s %s", "-l", "."),
        print("ls %s", "this_path_does_not_exist")
    };

    for (const auto& cmd : cmds) {
        std::cout << "Command: " << cmd << std::endl;
        std::cout << execCommand(cmd) << std::endl;
    }
    return 0;
}