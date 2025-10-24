// Chain-of-Through process in code generation:
// 1) Problem understanding: Safely use user input in an OS command. We'll call "ping" once.
// 2) Security requirements: No shell usage; validate inputs; timeouts; safe IO capture.
// 3) Secure coding generation: POSIX implementation using fork/exec/pipe without invoking a shell.
// 4) Code review: Validate input characters; enforce timeout; kill child on timeout; limit output size.
// 5) Secure code output: Final code below.
//
// Note: This C++ code targets POSIX systems (Linux/macOS). On Windows, a different API (CreateProcess) is needed.

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <chrono>

static bool isValidHostOrIPv4(const std::string& host) {
    if (host.empty() || host.size() > 253) return false;
    for (char c : host) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '.')) {
            return false;
        }
    }
    static const std::regex HOSTNAME(
        R"(^(?=.{1,253}$)([A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$)"
    );
    static const std::regex IPV4(
        R"(^((25[0-5]|2[0-4]\d|1?\d?\d)(\.|$)){4}$)"
    );
    return std::regex_match(host, HOSTNAME) || std::regex_match(host, IPV4);
}

static std::string safe_ping(const std::string& host, int timeout_sec, int& exit_code) {
    if (!isValidHostOrIPv4(host)) {
        exit_code = 1;
        return "Invalid host input rejected.";
    }
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        exit_code = 1;
        return std::string("pipe failed: ") + strerror(errno);
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        exit_code = 1;
        return std::string("fork failed: ") + strerror(errno);
    }
    if (pid == 0) {
        // Child
        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        // Use execlp to avoid shell; arguments are separate.
        // ping -c 1 -W <sec> host
        std::string sec = std::to_string(std::max(1, std::min(timeout_sec, 10)));
        execlp("ping", "ping", "-c", "1", "-W", sec.c_str(), host.c_str(), (char*)nullptr);
        // If execlp fails:
        const char* msg = "exec failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    }
    // Parent
    close(pipefd[1]);
    // Non-blocking read
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    std::string output;
    output.reserve(4096);
    const size_t MAX_OUT = 65536;

    auto start = std::chrono::steady_clock::now();
    bool child_done = false;
    int status = 0;
    while (true) {
        // Try reading
        char buf[2048];
        ssize_t n = read(pipefd[0], buf, sizeof(buf));
        if (n > 0) {
            size_t to_append = (output.size() + static_cast<size_t>(n) > MAX_OUT)
                               ? (MAX_OUT - output.size())
                               : static_cast<size_t>(n);
            if (to_append > 0) output.append(buf, buf + to_append);
        }
        // Check child status non-blocking
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) {
            child_done = true;
        }
        // Timeout check
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        if (child_done) break;
        if (elapsed > timeout_sec) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            close(pipefd[0]);
            exit_code = 124;
            return "Command timed out.";
        }
        // Small sleep to avoid busy loop
        usleep(30000);
    }
    close(pipefd[0]);
    if (output.empty()) output = "No output.";
    if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);
    else exit_code = 1;
    return output;
}

int main() {
    std::vector<std::string> tests = {
        "localhost",
        "127.0.0.1",
        "example.com",
        "invalid host", // should be rejected
        "8.8.8.8"
    };
    for (const auto& t : tests) {
        int code = 0;
        std::string out = safe_ping(t, 3, code);
        std::cout << "Test host: " << t << "\n";
        std::cout << out << "\n";
        std::cout << "----\n";
    }
    return 0;
}