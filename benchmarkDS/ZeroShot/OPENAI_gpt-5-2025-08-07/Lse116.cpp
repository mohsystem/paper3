#include <iostream>
#include <string>
#include <cctype>
#include <cstdarg>
#include <vector>
#include <cstdio>
#include <cstdlib>

// Validate path to avoid command injection
bool isSafePath(const std::string& s) {
    if (s.empty()) return false;
    for (unsigned char c : s) {
        bool ok = std::isalnum(c) || c == '/' || c == '.' || c == '_' || c == '-' || c == '+';
        if (!ok) return false;
    }
    return true;
}

// Print-like formatting function that returns a std::string
std::string vprint_format(const char* fmt, va_list ap) {
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0) return std::string();
    std::string buf;
    buf.resize(static_cast<size_t>(needed));
    std::vsnprintf(&buf[0], buf.size() + 1, fmt, ap);
    return buf;
}

std::string print_format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string out = vprint_format(fmt, ap);
    va_end(ap);
    return out;
}

// Execute system command
int system_exec(const std::string& cmd) {
    int rc = std::system(cmd.c_str());
    return rc;
}

// Function to execute ls with a sanitized path argument
int ls_command(const std::string& dir) {
    if (!isSafePath(dir)) {
        std::cout << "Rejected unsafe path: " << dir << "\n";
        return -1;
    }
    std::string cmd = print_format("/bin/ls -la -- %s", dir.c_str());
    return system_exec(cmd);
}

int main() {
    std::vector<std::string> tests = {
        ".",
        "src",
        "/tmp",
        "does-not-exist",
        "bad;rm -rf /"
    };
    for (const auto& t : tests) {
        std::cout << "Running ls on: " << t << "\n";
        int status = ls_command(t);
        std::cout << "Exit status: " << status << "\n";
        std::cout << "----\n";
    }
    return 0;
}