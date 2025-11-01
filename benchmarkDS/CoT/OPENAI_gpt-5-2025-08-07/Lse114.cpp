#include <iostream>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

// "print" function: creates a formatted string from a format and variable arguments
std::string vprint_fmt(const char* fmt, va_list ap) {
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0) return std::string();
    std::string out;
    out.resize(static_cast<size_t>(needed));
    std::vsnprintf(out.data(), out.size() + 1, fmt, ap);
    return out;
}

std::string print_fmt(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string s = vprint_fmt(fmt, ap);
    va_end(ap);
    return s;
}

// "system" function: executes a command and returns the exit code
int system_exec(const std::string& cmd) {
    return std::system(cmd.c_str());
}

// Convenience function combining print + system
int execf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string cmd = vprint_fmt(fmt, ap);
    va_end(ap);
    return system_exec(cmd);
}

// 5 test cases
int main() {
    int r1 = execf("ls");
    int r2 = execf("ls %s", "-l");
    int r3 = execf("ls %s %s", "-a", ".");
    int r4 = execf("ls %s", "--color=auto");
    int r5 = execf("ls %s", "..");

    std::cout << "Exit codes:\n";
    std::cout << r1 << "\n";
    std::cout << r2 << "\n";
    std::cout << r3 << "\n";
    std::cout << r4 << "\n";
    std::cout << r5 << "\n";
    return 0;
}