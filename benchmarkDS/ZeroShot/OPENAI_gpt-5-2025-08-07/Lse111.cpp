#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <cctype>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#endif

static bool isAllowedChars(const std::string& s) {
    if (s.empty() || s.size() > 253) return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='.' || c=='-' || c==':')) return false;
    }
    return true;
}

static bool isIPv4(const std::string& s) {
    static const std::regex re("^(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}$");
    return std::regex_match(s, re);
}

static bool isHostname(const std::string& s) {
    static const std::regex re("^(?=.{1,253}$)(?!-)[A-Za-z0-9-]{1,63}(?<!-)(\\.(?!-)[A-Za-z0-9-]{1,63}(?<!-))*\\.?$");
    return std::regex_match(s, re);
}

static bool isIPv6Like(const std::string& s) {
    if (s.find(':') == std::string::npos) return false;
    // Loose IPv6 sanity check
    int parts = 0;
    size_t start = 0, pos;
    while (true) {
        pos = s.find(':', start);
        std::string part = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);
        if (!part.empty()) {
            if (part.size() > 4) return false;
            for (char c : part) {
                if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
            }
        }
        parts++;
        if (pos == std::string::npos) break;
        start = pos + 1;
        if (parts > 8) return false;
    }
    return parts >= 2 && parts <= 8;
}

static bool isValidHost(const std::string& host) {
    if (!isAllowedChars(host)) return false;
    if (isIPv4(host)) return true;
    if (isHostname(host)) return true;
    if (isIPv6Like(host)) return true;
    return false;
}

std::string ping(const std::string& url) {
    std::string host = url;
    // trim
    host.erase(host.begin(), std::find_if(host.begin(), host.end(), [](int ch){return !std::isspace(ch);} ));
    host.erase(std::find_if(host.rbegin(), host.rend(), [](int ch){return !std::isspace(ch);} ).base(), host.end());

    if (!isValidHost(host)) {
        return "Invalid host";
    }

#ifdef _WIN32
    return "Ping not supported in this C++ demo on Windows";
#else
    std::vector<const char*> argv;
    argv.push_back("ping");
#if defined(__APPLE__) || defined(__FreeBSD__)
    // macOS/BSD: -c count, -W timeout(ms) not universally available; use -t (ttl) unavailable; fallback to -c only
    argv.push_back("-c");
    argv.push_back("2");
#else
    argv.push_back("-c");
    argv.push_back("2");
    argv.push_back("-W");
    argv.push_back("2");
#endif
    argv.push_back(host.c_str());
    argv.push_back(nullptr);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Failed to create pipe";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return "Failed to fork";
    } else if (pid == 0) {
        // Child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execvp("ping", (char* const*)argv.data());
        // If exec fails:
        const char* msg = "exec failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    std::string output;
    output.reserve(4096);

    // Use select for timeout
    const int TIMEOUT_SEC = 8;
    bool timedOut = false;
    int status = 0;

    while (true) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(pipefd[0], &rfds);
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SEC;
        tv.tv_usec = 0;
        int sel = select(pipefd[0] + 1, &rfds, nullptr, nullptr, &tv);
        if (sel > 0 && FD_ISSET(pipefd[0], &rfds)) {
            char buf[4096];
            ssize_t n = read(pipefd[0], buf, sizeof(buf));
            if (n > 0) {
                output.append(buf, buf + n);
                if (output.size() > 100000) {
                    output.append("\n[truncated]");
                    break;
                }
                continue;
            } else {
                // EOF
                break;
            }
        } else if (sel == 0) {
            timedOut = true;
            kill(pid, SIGKILL);
            break;
        } else {
            // error
            break;
        }
    }

    close(pipefd[0]);
    waitpid(pid, &status, 0);
    if (timedOut) {
        output.append("\n[Timed out]");
    }
    return output;
#endif
}

int main() {
    std::vector<std::string> tests = {
        "localhost",
        "127.0.0.1",
        "::1",
        "example.com",
        "invalid_host!"
    };
    for (const auto& t : tests) {
        std::cout << "== ping(" << t << ") ==" << std::endl;
        std::cout << ping(t) << std::endl;
    }
    return 0;
}