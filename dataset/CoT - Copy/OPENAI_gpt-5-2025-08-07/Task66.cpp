// Chain-of-Through steps applied:
// 1) Understand: validate and run limited bash-like command without shell, capture output.
// 2) Security: disallow shell, forbid meta chars, allowlist commands/args, limit time/output.
// 3) Implement: fork+execv with absolute binaries, non-blocking read, timeout, strict token checks.
// 4) Review: no system/popen, environment not passed from user, safe paths, bounded buffers.
// 5) Finalize: Provide function and 5 test cases.

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

static const int MAX_CMD_LEN_CPP = 256;
static const int MAX_ARGS_CPP = 8;
static const size_t MAX_OUTPUT_BYTES_CPP = 1000000;
static const int TIMEOUT_SEC_CPP = 3;

static const std::unordered_set<std::string> ALLOWED_CMDS_CPP = {
    "echo", "ls", "pwd", "whoami", "date", "uname", "cat"
};

static bool hasForbiddenChars(const std::string& s) {
    const std::string forbidden = ";|&><`$(){}[]\\'\"*!?~\r\n\t";
    for (unsigned char c : s) {
        if (c < 32 || c > 126) return true;
        if (forbidden.find((char)c) != std::string::npos) return true;
    }
    return false;
}

static bool isSafeToken(const std::string& t) {
    if (t.empty() || t.size() > 128) return false;
    for (unsigned char c : t) {
        if (c < 32 || c > 126) return false;
        if (std::string(";|&><`$(){}[]\\'\"*!?~\r\n\t").find((char)c) != std::string::npos) return false;
        if (!(std::isalnum(c) || c=='.' || c=='_' || c=='/' || c=='-' ||
              c=='=' || c==':' || c=='@' || c==',' || c=='+' || c=='%')) {
            return false;
        }
    }
    return true;
}

static bool isSafePathArg(const std::string& t) {
    if (!isSafeToken(t)) return false;
    if (!t.empty() && t[0] == '/') return false;
    if (!t.empty() && t[0] == '-') return false;
    if (t.find("..") != std::string::npos) return false;
    return true;
}

static bool isLsFlag(const std::string& t) {
    if (t.size() < 2 || t[0] != '-') return false;
    for (size_t i = 1; i < t.size(); ++i) {
        char c = t[i];
        if (std::string("lah1").find(c) == std::string::npos) return false;
    }
    return true;
}

static std::vector<std::string> tokenize(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tok;
    std::string item;
    while (iss >> item) tok.push_back(item);
    return tok;
}

static bool validateTokens(const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens.size() > (size_t)MAX_ARGS_CPP) return false;
    std::string cmd = tokens[0];
    if (ALLOWED_CMDS_CPP.find(cmd) == ALLOWED_CMDS_CPP.end()) return false;
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    if (cmd == "pwd" || cmd == "whoami" || cmd == "date") {
        return args.empty();
    } else if (cmd == "uname") {
        return args.empty() || (args.size() == 1 && args[0] == "-a");
    } else if (cmd == "echo") {
        if (args.size() > 5) return false;
        for (auto& a : args) if (!isSafeToken(a)) return false;
        return true;
    } else if (cmd == "ls") {
        if (args.size() > 2) return false;
        bool sawPath = false;
        for (auto& a : args) {
            if (!a.empty() && a[0] == '-') {
                if (!isLsFlag(a)) return false;
            } else {
                if (sawPath) return false;
                if (!isSafePathArg(a)) return false;
                sawPath = true;
            }
        }
        return true;
    } else if (cmd == "cat") {
        if (args.empty() || args.size() > 2) return false;
        for (auto& a : args) if (!isSafePathArg(a)) return false;
        return true;
    }
    return false;
}

static std::string resolveAbsolute(const std::string& cmd) {
    std::vector<std::string> paths = {"/bin/" + cmd, "/usr/bin/" + cmd};
    for (auto& p : paths) {
        if (access(p.c_str(), X_OK) == 0) return p;
    }
    return "";
}

static std::string readFromFdNonBlocking(int fd, pid_t childPid) {
    // Set non-blocking
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    std::string out;
    out.reserve(8192);

    auto deadline = time(nullptr) + TIMEOUT_SEC_CPP;

    char buf[4096];
    bool childExited = false;

    while (true) {
        // Read loop
        ssize_t n;
        do {
            n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                if (out.size() + (size_t)n > MAX_OUTPUT_BYTES_CPP) {
                    out.append("\n[Truncated output]");
                    // Kill child if still running
                    if (!childExited) kill(childPid, SIGKILL);
                    // Drain wait
                    int status;
                    waitpid(childPid, &status, 0);
                    return out;
                }
                out.append(buf, buf + n);
            }
        } while (n > 0);

        // Check child status
        int status = 0;
        pid_t r = waitpid(childPid, &status, WNOHANG);
        if (r == childPid) {
            childExited = true;
            // Drain remaining data quickly
            while ((n = read(fd, buf, sizeof(buf))) > 0) {
                if (out.size() + (size_t)n > MAX_OUTPUT_BYTES_CPP) {
                    out.append("\n[Truncated output]");
                    break;
                }
                out.append(buf, buf + n);
            }
            break;
        }

        // Timeout?
        if (time(nullptr) > deadline) {
            kill(childPid, SIGKILL);
            waitpid(childPid, &status, 0);
            return std::string("[Error] Execution timed out");
        }

        // Small sleep to avoid busy wait
        usleep(20000);
    }
    return out;
}

std::string run_safe_command(const std::string& input) {
    if (input.empty()) return "Error: Empty input";
    if ((int)input.size() > MAX_CMD_LEN_CPP) return "Error: Command too long";
    if (hasForbiddenChars(input)) return "Error: Forbidden/invalid characters present";

    std::vector<std::string> tokens = tokenize(input);
    if (!validateTokens(tokens)) return "Error: Command not allowed or invalid arguments";

    std::string absbin = resolveAbsolute(tokens[0]);
    if (absbin.empty()) return "Error: Command binary not found";
    tokens[0] = absbin;

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return "Error: Pipe failed";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return "Error: Fork failed";
    } else if (pid == 0) {
        // Child
        // Redirect stdout and stderr
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        // Build argv
        std::vector<char*> argv;
        argv.reserve(tokens.size() + 1);
        for (auto& t : tokens) argv.push_back(const_cast<char*>(t.c_str()));
        argv.push_back(nullptr);

        // Clear possibly dangerous env
        unsetenv("LD_PRELOAD");
        unsetenv("LD_LIBRARY_PATH");

        execv(argv[0], argv.data());
        // If exec fails:
        const char* msg = "Execution failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    } else {
        // Parent
        close(pipefd[1]);
        std::string output = readFromFdNonBlocking(pipefd[0], pid);
        close(pipefd[0]);
        return output;
    }
}

int main() {
    std::vector<std::string> tests = {
        "pwd",
        "whoami",
        "date",
        "echo safe_test",
        "ls -l"
    };
    for (auto& t : tests) {
        std::cout << ">>> " << t << std::endl;
        std::cout << run_safe_command(t) << std::endl;
    }
    return 0;
}