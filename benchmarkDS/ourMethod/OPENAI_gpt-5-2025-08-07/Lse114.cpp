#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

static const std::vector<std::string> ALLOWED_FLAGS = {
    "", "-l", "-a", "-la", "-al", "-1", "-lh", "-lah", "-alh"
};

// "print" function: creates a formatted string from a format and variable args
std::string vprint_format(const char* fmt, ...) {
    if (!fmt) return std::string();
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0) {
        va_end(ap);
        return std::string();
    }
    std::string out;
    out.resize(static_cast<size_t>(needed));
    std::vsnprintf(&out[0], static_cast<size_t>(needed) + 1, fmt, ap);
    va_end(ap);
    return out;
}

bool isAllowedFlag(const std::string& f) {
    for (const auto& s : ALLOWED_FLAGS) {
        if (s == f) return true;
    }
    return false;
}

bool isSafePath(const std::string& path) {
    if (path.empty() || path.size() > 1024) return false;
    for (char c : path) {
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '/' || c == '-';
        if (!ok) return false;
    }
    return true;
}

// "system" function: executes a command string
int systemExec(const std::string& cmd) {
    if (cmd.empty()) return -1;
    return std::system(cmd.c_str());
}

// Secure wrapper to build and execute an ls command
int safe_ls(const std::string& flagsIn, const std::string& pathIn) {
    std::string flags = flagsIn;
    std::string path = pathIn.empty() ? "." : pathIn;

    if (!isAllowedFlag(flags)) {
        std::cerr << "Invalid flags\n";
        return -1;
    }
    if (!isSafePath(path)) {
        std::cerr << "Invalid path\n";
        return -1;
    }

    std::string cmd;
    if (!flags.empty()) {
        cmd = vprint_format("ls %s %s", flags.c_str(), path.c_str());
    } else {
        cmd = vprint_format("ls %s", path.c_str());
    }
    return systemExec(cmd);
}

int main() {
    // 5 test cases
    int rc1 = safe_ls("", ".");
    std::cout << "Return code: " << rc1 << "\n";

    int rc2 = safe_ls("-l", ".");
    std::cout << "Return code: " << rc2 << "\n";

    int rc3 = safe_ls("-a", "/");
    std::cout << "Return code: " << rc3 << "\n";

    int rc4 = safe_ls("-lh", ".");
    std::cout << "Return code: " << rc4 << "\n";

    int rc5 = safe_ls("-rf", "."); // invalid
    std::cout << "Return code: " << rc5 << "\n";
    return 0;
}