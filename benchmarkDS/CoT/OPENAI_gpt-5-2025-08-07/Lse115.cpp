#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstdio>

std::string print_fmt(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args2;
    va_copy(args2, args);
    int len = std::vsnprintf(nullptr, 0, fmt, args);
    va_end(args);
    if (len < 0) {
        va_end(args2);
        return "";
    }
    std::vector<char> buf(static_cast<size_t>(len) + 1);
    std::vsnprintf(buf.data(), buf.size(), fmt, args2);
    va_end(args2);
    return std::string(buf.data());
}

std::string exec_command(const std::string& cmd) {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

int main() {
    std::vector<std::string> cmds = {
        print_fmt("ls"),
        print_fmt("ls %s", "-l"),
        print_fmt("ls %s %s", "-a", "."),
        print_fmt("ls %s %s", "-1", ".."),
        print_fmt("ls %s %s %s", "-l", "-a", ".")
    };
    for (size_t i = 0; i < cmds.size(); ++i) {
        std::cout << "Command #" << (i + 1) << ": " << cmds[i] << "\n";
        std::cout << "Output:\n" << exec_command(cmds[i]) << "\n";
    }
    return 0;
}