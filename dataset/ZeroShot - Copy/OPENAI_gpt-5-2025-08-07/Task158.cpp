#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>

static const size_t OUTPUT_LIMIT = 8192;
static const int TIMEOUT_MS = 5000;

static bool is_valid_host(const std::string& s) {
    if (s.empty() || s.size() > 253) return false;
    static const std::regex re("^[A-Za-z0-9.-]+$");
    return std::regex_match(s, re);
}

std::string run_ping(const std::string& user_input) {
    if (!is_valid_host(user_input)) {
        return "Invalid input";
    }

    std::vector<const char*> args;
    args.push_back("ping");
    args.push_back("-c");
    args.push_back("1");
    args.push_back(user_input.c_str());
    args.push_back(nullptr);

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return "Execution failed";
    }

    // Set close-on-exec
    fcntl(pipefd[0], F_SETFD, fcntl(pipefd[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, fcntl(pipefd[1], F_GETFD) | FD_CLOEXEC);

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "Execution failed";
    } else if (pid == 0) {
        // Child
        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(args[0], const_cast<char* const*>(args.data()));
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    // Non-blocking read
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    if (flags >= 0) fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    std::string output;
    output.reserve(1024);
    struct pollfd pfd;
    pfd.fd = pipefd[0];
    pfd.events = POLLIN | POLLHUP | POLLERR;

    int elapsed = 0;
    const int step = 100; // ms
    bool pipe_open = true;
    bool child_exited = false;

    while (elapsed < TIMEOUT_MS) {
        int ret = poll(&pfd, 1, step);
        if (ret > 0) {
            if (pfd.revents & (POLLIN)) {
                char buf[512];
                ssize_t r = read(pipefd[0], buf, sizeof(buf));
                if (r > 0) {
                    size_t remain = OUTPUT_LIMIT - output.size();
                    if (remain > 0) {
                        output.append(buf, buf + std::min(static_cast<size_t>(r), remain));
                    }
                } else if (r == 0) {
                    pipe_open = false;
                }
            }
            if (pfd.revents & (POLLHUP | POLLERR)) {
                // Try to drain remaining
                char buf[512];
                ssize_t r;
                while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
                    size_t remain = OUTPUT_LIMIT - output.size();
                    if (remain > 0) {
                        output.append(buf, buf + std::min(static_cast<size_t>(r), remain));
                    }
                }
                pipe_open = false;
            }
        }
        int status = 0;
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) child_exited = true;

        if (!pipe_open && child_exited) break;
        elapsed += step;
    }

    close(pipefd[0]);

    if (!( !pipe_open && child_exited )) {
        // Timeout: kill the child
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        return "Timed out";
    }

    return output;
}

int main() {
    std::vector<std::string> tests = {
        "localhost",
        "127.0.0.1",
        "example.com",
        "bad;rm -rf /",
        "name with spaces"
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t << "\n";
        std::cout << "Output (truncated):\n";
        std::cout << run_ping(t) << "\n";
        std::cout << "----\n";
    }
    return 0;
}