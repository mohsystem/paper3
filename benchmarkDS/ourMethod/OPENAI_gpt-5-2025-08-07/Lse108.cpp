#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>

static bool validate_username(const std::string& u) {
    if (u.empty() || u.size() > 32) return false;
    for (unsigned char c : u) {
        if (std::isalnum(c)) continue;
        if (c == '.' || c == '_' || c == '-') continue;
        return false;
    }
    return true;
}

// Executes "getent passwd <username>" via system() and directs stdout to 'output'.
// Returns the child's exit code (0 if found) or negative on validation/error.
int getent_to_fp_cpp(const std::string& username, FILE* output) {
    if (!output) return -1;
    if (!validate_username(username)) return -1;

    int fd = fileno(output);
    if (fd < 0) return -1;

    // Clear FD_CLOEXEC to ensure the descriptor is inherited by the shell.
    int old_flags = fcntl(fd, F_GETFD);
    if (old_flags == -1) return -1;
    bool restore_needed = false;
    if (old_flags & FD_CLOEXEC) {
        if (fcntl(fd, F_SETFD, old_flags & ~FD_CLOEXEC) == -1) {
            return -1;
        }
        restore_needed = true;
    }

    char cmd[512];
    // Build: getent passwd <username> 1>&<fd> 2>/dev/null
    int n = snprintf(cmd, sizeof(cmd), "getent passwd %s 1>&%d 2>/dev/null", username.c_str(), fd);
    if (n < 0 || static_cast<size_t>(n) >= sizeof(cmd)) {
        if (restore_needed) fcntl(fd, F_SETFD, old_flags);
        return -1;
    }

    int status = system(cmd);

    if (restore_needed) {
        // Best-effort restore
        (void)fcntl(fd, F_SETFD, old_flags);
    }

    // Normalize exit code
    if (status == -1) return -1;
#ifdef WIFEXITED
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
#endif
    return -1;
}

int main() {
    std::vector<std::string> tests = {
        "root",
        "nobody",
        "daemon",
        "thisuserdoesnotexist",
        "bad;injection" // invalid by validation
    };

    for (const auto& u : tests) {
        // Create a secure temporary file
        char tmpl[] = "/tmp/getent_cpp_XXXXXX";
        int tfd = mkstemp(tmpl);
        if (tfd == -1) {
            std::perror("mkstemp");
            return 1;
        }
        // Unlink immediately to avoid leaving files around; the fd remains valid.
        unlink(tmpl);
        FILE* f = fdopen(tfd, "w+");
        if (!f) {
            std::perror("fdopen");
            close(tfd);
            return 1;
        }

        int rc = getent_to_fp_cpp(u, f);
        fflush(f);
        fseek(f, 0, SEEK_SET);
        std::cout << "User: " << u << " -> rc=" << rc << "\n";
        char buf[4096];
        size_t r;
        bool any = false;
        while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
            any = true;
            std::cout.write(buf, r);
        }
        if (!any) {
            std::cout << "(no output)\n";
        }
        std::cout << "----\n";
        fclose(f);
    }
    return 0;
}