#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

std::string print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = std::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);
    std::vector<char> buf(needed + 1);
    std::vsnprintf(buf.data(), buf.size(), fmt, args);
    va_end(args);
    return std::string(buf.data());
}

int run_ls(const std::string& options, const std::string& directory) {
    std::string opt = options;
    std::string dir = directory.empty() ? "." : directory;
    std::string cmd = print("ls %s %s", opt.c_str(), dir.c_str());
    return std::system(cmd.c_str());
}

int main() {
    run_ls("", ".");
    run_ls("-l", ".");
    run_ls("-a", ".");
    run_ls("-la", ".");
    run_ls("", "..");
    return 0;
}