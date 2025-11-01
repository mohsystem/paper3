#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

static std::string sprint(const char* fmt, ...) {
    if (!fmt) throw std::invalid_argument("Format string cannot be null");
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        throw std::runtime_error("Formatting failed");
    }
    std::string s;
    s.resize(static_cast<size_t>(n));
    vsnprintf(&s[0], static_cast<size_t>(n) + 1, fmt, ap2);
    va_end(ap2);
    return s;
}

static bool isSafePath(const std::string& s) {
    if (s.empty() || s.size() > 4096) return false;
    if (s[0] == '-') return false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c==' ' || c=='/' || c=='.' || c=='_' || c=='-')) {
            return false;
        }
    }
    return true;
}

static std::string execute_ls(const std::string& arg) {
    std::string path = arg.empty() ? "." : arg;
    if (!isSafePath(path)) {
        return "Invalid or unsafe path input.\n";
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return std::string("pipe failed: ") + std::strerror(errno) + "\n";
    }

    // Set close-on-exec
    fcntl(pipefd[0], F_SETFD, fcntl(pipefd[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, fcntl(pipefd[1], F_GETFD) | FD_CLOEXEC);

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return std::string("fork failed: ") + std::strerror(errno) + "\n";
    }

    if (pid == 0) {
        // Child
        // Redirect stdout and stderr to pipe write end
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>("ls"));
        argv.push_back(const_cast<char*>("--color=never"));
        argv.push_back(const_cast<char*>("--"));
        argv.push_back(const_cast<char*>(path.c_str()));
        argv.push_back(nullptr);

        execvp("ls", argv.data());
        // If exec fails
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    std::string output;
    output.reserve(8192);
    const size_t MAX_OUT = 10 * 1024 * 1024; // 10MB
    char buf[8192];
    ssize_t r;
    size_t total = 0;
    while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (total + static_cast<size_t>(r) > MAX_OUT) {
            output.append(buf, buf + (MAX_OUT - total));
            output.append("\n[Output truncated]\n");
            // Drain remaining
            while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) { /* discard */ }
            break;
        } else {
            output.append(buf, buf + r);
            total += static_cast<size_t>(r);
        }
    }
    close(pipefd[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    (void)status;
    return output;
}

int main() {
    std::vector<std::string> tests = {
        sprint("%s", "."),
        sprint("%s", "/"),
        sprint("%s", ".."),
        sprint("./%s", "nonexistent_dir_hopefully"),
        sprint("%s-%s", "src", "main")
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " input: " << tests[i] << "\n";
        std::cout << execute_ls(tests[i]);
        std::cout << "----\n";
    }
    return 0;
}