#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// "print" function: creates a formatted std::string from a format and variable arguments.
std::string print(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);
    if (needed < 0) {
        va_end(ap_copy);
        return std::string();
    }
    std::vector<char> buf(static_cast<size_t>(needed) + 1);
    std::vsnprintf(buf.data(), buf.size(), fmt, ap_copy);
    va_end(ap_copy);
    return std::string(buf.data());
}

// "system" function: executes the provided command string.
int runSystem(const std::string& cmd) {
    return std::system(cmd.c_str());
}

// Convenience function: format and execute.
int runSystemf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);
    if (needed < 0) {
        va_end(ap_copy);
        return -1;
    }
    std::vector<char> buf(static_cast<size_t>(needed) + 1);
    std::vsnprintf(buf.data(), buf.size(), fmt, ap_copy);
    va_end(ap_copy);
    return runSystem(std::string(buf.data()));
}

int main() {
    // 5 test cases using ls with different arguments
    runSystemf("ls");
    runSystemf("ls %s", "-l");
    runSystemf("ls %s %s", "-l", "-a");
    runSystemf("ls %s %s %s", "-l", "-a", ".");
    runSystemf("ls %s", ".");
    return 0;
}