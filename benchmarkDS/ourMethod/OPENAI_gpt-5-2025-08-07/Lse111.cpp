#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#endif

static std::string trim(const std::string &s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j-1]))) j--;
    return s.substr(i, j - i);
}

static std::string extract_host(const std::string &inputUrl) {
    std::string s = trim(inputUrl);
    if (s.empty() || s.size() > 2048) return "";
    // Try to locate scheme://
    size_t start = 0;
    size_t scheme = s.find("://");
    if (scheme != std::string::npos) {
        start = scheme + 3;
    }
    // Take until '/', '?', '#'
    size_t end = s.find_first_of("/?#", start);
    if (end == std::string::npos) end = s.size();
    std::string host = s.substr(start, end - start);
    if (host.size() >= 2 && host.front() == '[' && host.back() == ']') {
        host = host.substr(1, host.size() - 2);
    }
    return host;
}

static bool is_valid_host(const std::string &host) {
    if (host.empty() || host.size() > 253) return false;
    if (host.front() == '.' || host.back() == '.' || host.front() == '-' || host.back() == '-') return false;
    if (host.find("..") != std::string::npos) return false;
    for (char c : host) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='.' || c=='-' || c==':')) {
            return false;
        }
    }
#ifndef _WIN32
    // If it looks like IPv6
    if (host.find(':') != std::string::npos) {
        struct in6_addr a6;
        if (inet_pton(AF_INET6, host.c_str(), &a6) == 1) return true;
        return false;
    }
    // If IPv4
    struct in_addr a4;
    if (inet_pton(AF_INET, host.c_str(), &a4) == 1) return true;
#endif
    // hostname validation
    size_t i = 0;
    while (i < host.size()) {
        size_t j = host.find('.', i);
        if (j == std::string::npos) j = host.size();
        size_t len = j - i;
        if (len == 0 || len > 63) return false;
        if (!std::isalnum(static_cast<unsigned char>(host[i])) ||
            !std::isalnum(static_cast<unsigned char>(host[j-1]))) return false;
        for (size_t k = i; k < j; ++k) {
            char c = host[k];
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-')) return false;
        }
        i = (j == host.size()) ? j : j + 1;
    }
    return true;
}

static std::string run_ping(const std::string &host) {
#ifdef _WIN32
    (void)host;
    return "Ping not implemented for Windows in this demo";
#else
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return "Ping failed";
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return "Ping failed";
    } else if (pid == 0) {
        // Child
        close(pipefd[0]);
        // Redirect stdout and stderr
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        // Close extra
        for (int fd = 3; fd < 256; ++fd) {
            if (fd != pipefd[1]) close(fd);
        }
        execlp("ping", "ping", "-n", "-c", "1", "-W", "1", host.c_str(), (char*)NULL);
        // If exec fails
        const char *msg = "Ping failed to exec\n";
        write(pipefd[1], msg, std::strlen(msg));
        _exit(127);
    } else {
        // Parent
        close(pipefd[1]);
        const size_t MAX_OUT = 65536;
        std::string out;
        out.reserve(4096);
        char buf[1024];
        ssize_t r;
        while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
            size_t can_take = (out.size() + static_cast<size_t>(r) > MAX_OUT)
                                ? (MAX_OUT - out.size())
                                : static_cast<size_t>(r);
            if (can_take > 0) out.append(buf, buf + can_take);
            if (out.size() >= MAX_OUT) break;
        }
        close(pipefd[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        return out.empty() ? std::string("No output") : out;
    }
#endif
}

std::string ping(const std::string &inputUrl) {
    std::string host = extract_host(inputUrl);
    if (!is_valid_host(host)) {
        return "Invalid host";
    }
    return run_ping(host);
}

int main() {
    std::vector<std::string> tests{
        "http://127.0.0.1",
        "https://localhost",
        "https://example.com",
        "http://[::1]/",
        "http://bad_host_$$$"
    };
    for (const auto &t : tests) {
        std::cout << "=== Test: " << t << " ===\n";
        std::string out = ping(t);
        if (out.size() > 500) {
            std::cout << out.substr(0, 500) << "...(truncated)\n\n";
        } else {
            std::cout << out << "\n";
        }
    }
    return 0;
}