#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>

static bool is_valid_username(const std::string& u) {
    if (u.empty() || u.size() > 32) return false;
    unsigned char c0 = static_cast<unsigned char>(u[0]);
    if (!(std::isalnum(c0) || c0 == '_')) return false;
    for (size_t i = 1; i < u.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(u[i]);
        if (!(std::isalnum(c) || c == '_' || c == '-' || c == '.')) return false;
    }
    return true;
}

int get_user_info_system(const std::string& username, std::FILE* output) {
    if (!output) return 400;
    if (!is_valid_username(username)) return 400;

    int fd = fileno(output);
    if (fd < 0) return 400;

    fflush(output);

    // Absolute path to getent, redirect stdout and stderr to the provided fd.
    // Use a bounded buffer to avoid overflow.
    char cmd[256];
    // Ensure PATH is constrained for safety in the shell environment
    // Note: system() uses /bin/sh -c, which will inherit PATH.
    setenv("PATH", "/usr/bin:/bin", 1);

    int n = std::snprintf(cmd, sizeof(cmd),
                          "/usr/bin/getent passwd %s 1>&%d 2>&1",
                          username.c_str(), fd);
    if (n < 0 || static_cast<size_t>(n) >= sizeof(cmd)) {
        return 400;
    }

    int rc = std::system(cmd);
    return rc;
}

int main() {
    const char* tests[] = {"root", "nobody", "daemon", "thisuserdoesnotexist12345", "bin"};
    for (const char* t : tests) {
        std::FILE* f = std::tmpfile();
        if (!f) {
            std::perror("tmpfile");
            continue;
        }
        int rc = get_user_info_system(t, f);
        std::cout << "=== C++ Test for '" << t << "' (rc=" << rc << ") ===\n";
        std::rewind(f);
        char buf[512];
        bool any = false;
        while (std::fgets(buf, sizeof(buf), f)) {
            any = true;
            std::fputs(buf, stdout);
        }
        if (!any) {
            std::cout << "(no output)\n";
        }
        std::fclose(f);
    }
    return 0;
}