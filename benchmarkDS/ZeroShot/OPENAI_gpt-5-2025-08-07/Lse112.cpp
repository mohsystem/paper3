#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <chrono>
#include <thread>

static bool is_valid_host(const std::string& url) {
    static const std::regex ipv4(R"(^(?:(?:25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.){3}(?:25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)$)");
    static const std::regex host(R"(^(?=.{1,253}$)(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\.)+[A-Za-z]{2,63}$)");
    std::string u;
    u.reserve(url.size());
    for (char c : url) {
        if (c == '\n' || c == '\r' || c == '\0') return false;
        u.push_back(c);
    }
    if (u == "localhost" || u == "LOCALHOST") return true;
    if (std::regex_match(u, ipv4)) return true;
    return std::regex_match(u, host);
}

// Returns the URL on success, empty string on failure.
std::string secure_ping(const std::string& url) {
    if (!is_valid_host(url)) {
        return "";
    }
    // Build args for Unix-like systems
    const char* prog = "ping";
    const char* args[] = {"ping", "-c", "1", "-W", "2", url.c_str(), nullptr};

    pid_t pid = fork();
    if (pid < 0) {
        return "";
    } else if (pid == 0) {
        // Child
        // Detach from any inherited fds except stdio
        // Optionally redirect stdout/stderr to /dev/null
        int fd = ::open("/dev/null", O_WRONLY);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (fd > STDERR_FILENO) close(fd);
        }
        execvp(prog, (char* const*)args);
        _exit(127);
    } else {
        // Parent: wait with timeout
        int status = 0;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (std::chrono::steady_clock::now() < deadline) {
            pid_t r = waitpid(pid, &status, WNOHANG);
            if (r == pid) {
                return url;
            } else if (r < 0) {
                return "";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        return "";
    }
}

int main() {
    std::vector<std::string> tests = {
        "example.com",
        "127.0.0.1",
        "8.8.8.8",
        "invalid;;host",
        "localhost"
    };
    for (const auto& t : tests) {
        std::string res = secure_ping(t);
        std::cout << "Input: " << t << " -> Returned: " << (res.empty() ? "null" : res) << std::endl;
    }
    return 0;
}