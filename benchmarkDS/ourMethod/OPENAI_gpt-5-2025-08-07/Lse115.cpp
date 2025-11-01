#include <cerrno>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cctype>

static bool is_safe_path(const std::string& s) {
    if (s.size() > 255) return false;
    if (s.find('\0') != std::string::npos) return false;
    if (s.find("..") != std::string::npos) return false;
    for (unsigned char ch : s) {
        if (!(std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-' || ch == '/')) {
            return false;
        }
    }
    return true;
}

static std::string build_string(const char* fmt, ...) {
    if (fmt == nullptr) return std::string();
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int needed = vsnprintf(nullptr, 0, fmt, ap2);
    va_end(ap2);
    if (needed < 0 || needed > 255) {
        va_end(ap);
        return std::string();
    }
    std::string out;
    out.resize(static_cast<size_t>(needed));
    int written = vsnprintf(&out[0], static_cast<size_t>(needed) + 1, fmt, ap);
    va_end(ap);
    if (written < 0 || written > needed) {
        return std::string();
    }
    if (!is_safe_path(out)) return std::string();
    return out;
}

static std::string execute_ls(const std::string& input) {
    std::string path = input.empty() ? "." : input;
    if (!is_safe_path(path)) {
        return std::string("error: invalid input\n");
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return std::string("error: pipe\n");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return std::string("error: fork\n");
    }

    if (pid == 0) {
        // Child
        // Redirect stdout to pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(127);
        }
        // Redirect stderr to stdout to capture all output
        if (dup2(pipefd[1], STDERR_FILENO) == -1) {
            _exit(127);
        }
        close(pipefd[0]);
        close(pipefd[1]);

        const char* argv_child[] = {"ls", "--", path.c_str(), nullptr};
        execvp("ls", const_cast<char* const*>(argv_child));
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    std::string output;
    char buf[4096];
    ssize_t n = 0;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        output.append(buf, buf + n);
    }
    close(pipefd[0]);
    int status = 0;
    (void)waitpid(pid, &status, 0);
    return output;
}

int main() {
    std::vector<std::string> tests;
    tests.push_back(build_string("%s", "."));               // current directory
    tests.push_back(build_string("%s", "/"));               // root
    tests.push_back(build_string("nonexistent_%s", "dir")); // likely nonexistent
    tests.push_back(build_string("%s/%s", "tmp", "test"));  // nested
    tests.push_back(build_string("%s%s", ".", ""));         // another current dir

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "=== Test " << (i + 1) << " ===" << std::endl;
        std::string input = tests[i];
        std::string out = execute_ls(input);
        std::cout << out;
    }
    return 0;
}