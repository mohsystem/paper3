#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

static std::string print_format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int n = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (n < 0) {
        va_end(ap);
        return std::string();
    }
    std::string out;
    out.resize(static_cast<size_t>(n) + 1);
    std::vsnprintf(&out[0], out.size(), fmt, ap);
    va_end(ap);
    out.pop_back(); // remove trailing null
    return out;
}

static std::string shell_quote(const std::string& s) {
    if (s.empty()) {
        return "''";
    }
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('\'');
    for (char c : s) {
        if (c == '\'') {
            out.append("'\\''");
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

static int run_ls(const std::string& path) {
    const char* base = "/bin/ls -la";
    std::string cmd;
    if (!path.empty()) {
        std::string q = shell_quote(path);
        cmd = print_format("%s %s", base, q.c_str());
    } else {
        cmd = base;
    }
    return std::system(cmd.c_str());
}

int main() {
    const char* tests[] = { nullptr, ".", "/", "/tmp", "no_such_file_xyz" };
    for (const char* t : tests) {
        std::cout << "== C++ run_ls(" << (t ? t : "null/empty") << ") ==" << std::endl;
        std::string arg = t ? std::string(t) : std::string();
        run_ls(arg);
    }
    return 0;
}