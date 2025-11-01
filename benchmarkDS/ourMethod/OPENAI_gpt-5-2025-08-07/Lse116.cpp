#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <iostream>

// Safe formatter (print-like) to build a command string.
static bool vprint_format(std::string &out, const char *fmt, va_list ap) {
    if (!fmt) {
        out.clear();
        return false;
    }
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0 || needed > 256) { // enforce max length
        out.clear();
        return false;
    }
    out.resize(static_cast<size_t>(needed));
    int written = std::vsnprintf(&out[0], static_cast<size_t>(needed) + 1, fmt, ap);
    if (written != needed) {
        out.clear();
        return false;
    }
    // Trim spaces
    while (!out.empty() && (out.front() == ' ' || out.front() == '\t' || out.front() == '\n' || out.front() == '\r')) {
        out.erase(out.begin());
    }
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t' || out.back() == '\n' || out.back() == '\r')) {
        out.pop_back();
    }
    return !out.empty();
}

static bool print_format(std::string &out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    bool ok = vprint_format(out, fmt, ap);
    va_end(ap);
    return ok;
}

// Validate command is a safe "ls" invocation with at most one simple argument
static bool is_safe_ls_command(const std::string &cmd) {
    if (cmd.size() < 2 || cmd.size() > 256) return false;
    if (cmd.rfind("ls", 0) != 0) return false; // must start with "ls"
    if (cmd.size() == 2) return true; // exactly "ls"
    if (cmd.size() < 4) return false; // needs space and at least one char
    if (cmd[2] != ' ') return false;
    std::string arg = cmd.substr(3);
    if (arg.empty() || arg.size() > 200) return false;
    if (arg.find(' ') != std::string::npos) return false; // no spaces in arg
    if (arg.find("..") != std::string::npos) return false; // block traversal
    for (char c : arg) {
        bool ok = (std::isalnum(static_cast<unsigned char>(c)) != 0) || c == '/' || c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

// "system"-like function that executes the provided command string.
static int safe_system(const std::string &cmd) {
    if (!is_safe_ls_command(cmd)) {
        std::cout << "Rejected command\n";
        return -1;
    }
    int rc = std::system(cmd.c_str());
    return rc;
}

// Convenience function: format then execute ls.
int run_ls_formatted(const char *fmt, ...) {
    std::string cmd;
    va_list ap;
    va_start(ap, fmt);
    bool ok = vprint_format(cmd, fmt, ap);
    va_end(ap);
    if (!ok) {
        std::cout << "Rejected command\n";
        return -1;
    }
    return safe_system(cmd);
}

int main() {
    // 5 test cases
    int r1 = run_ls_formatted("ls");
    std::cout << "Exit: " << r1 << "\n";

    int r2 = run_ls_formatted("ls %s", ".");
    std::cout << "Exit: " << r2 << "\n";

    int r3 = run_ls_formatted("ls %s", "/");
    std::cout << "Exit: " << r3 << "\n";

    int r4 = run_ls_formatted("ls %s", "./");
    std::cout << "Exit: " << r4 << "\n";

    int r5 = run_ls_formatted("ls %s", "nonexistent_path_should_fail");
    std::cout << "Exit: " << r5 << "\n";

    return 0;
}