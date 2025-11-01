#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>

static bool is_valid_username(const std::string& u) {
    if (u.empty() || u.size() > 32) return false;
    auto isAlpha = [](char c){ return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
    auto isDigit = [](char c){ return (c >= '0' && c <= '9'); };
    if (!(isAlpha(u[0]) || u[0] == '_')) return false;
    for (size_t i = 1; i < u.size(); ++i) {
        char c = u[i];
        if (!(isAlpha(c) || isDigit(c) || c == '_' || c == '-')) return false;
    }
    return true;
}

std::string get_user_info(const std::string& username) {
    if (!is_valid_username(username)) {
        throw std::invalid_argument("Invalid username");
    }

    std::string pattern = "^" + username + ":";

    int pipe1[2]; // cat -> grep
    int pipe2[2]; // grep -> parent
    if (pipe(pipe1) == -1) {
        throw std::runtime_error("pipe failed");
    }
    if (pipe(pipe2) == -1) {
        close(pipe1[0]); close(pipe1[1]);
        throw std::runtime_error("pipe failed");
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        throw std::runtime_error("fork failed");
    }
    if (pid1 == 0) {
        // cat child
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        execl("/bin/cat", "cat", "/etc/passwd", (char*)NULL);
        _exit(127);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        int status;
        waitpid(pid1, &status, 0);
        throw std::runtime_error("fork failed");
    }
    if (pid2 == 0) {
        // grep child
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        // Note: const_cast is safe; exec does not modify argv
        execl("/bin/grep", "grep", "-E", pattern.c_str(), (char*)NULL);
        _exit(127);
    }

    // Parent
    close(pipe1[0]); close(pipe1[1]);
    close(pipe2[1]);

    const size_t MAX_READ = 2 * 1024 * 1024; // 2MB cap
    std::string output;
    output.reserve(256);
    char buf[8192];
    size_t total = 0;
    ssize_t r;
    while ((r = read(pipe2[0], buf, sizeof(buf))) > 0) {
        total += static_cast<size_t>(r);
        if (total > MAX_READ) {
            close(pipe2[0]);
            int st;
            waitpid(pid1, &st, 0);
            waitpid(pid2, &st, 0);
            throw std::runtime_error("output too large");
        }
        output.append(buf, buf + r);
    }
    close(pipe2[0]);

    int st1, st2;
    waitpid(pid1, &st1, 0);
    waitpid(pid2, &st2, 0);
    // grep exit code 1 means no match; we still return what we read (possibly empty)

    return output;
}

int main(int argc, char** argv) {
    std::vector<std::string> tests;
    if (argc > 1) {
        tests.push_back(argv[1]);
    } else {
        tests = {"root", "daemon", "bin", "nobody", "no_such_user_zzz"};
    }

    for (const auto& u : tests) {
        std::cout << "=== Query: " << u << " ===\n";
        try {
            std::string out = get_user_info(u);
            if (out.empty()) {
                std::cout << "(no match)\n";
            } else {
                std::cout << out;
                if (out.back() != '\n') std::cout << '\n';
            }
        } catch (...) {
            std::cout << "(error)\n";
        }
    }
    return 0;
}