#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

struct ExecResult {
    int exitCode;
    std::string stdoutStr;
    std::string stderrStr;
    std::string error;
};

static bool isAllowedChar(char c) {
    // Allowed: alnum and . _ - + / = % : , @
    if (std::isalnum(static_cast<unsigned char>(c))) return true;
    const std::string allowed = "._-+/=%:,@";
    return allowed.find(c) != std::string::npos;
}

static bool isSafeToken(const std::string &s, size_t maxLen = 128) {
    if (s.empty() || s.size() > maxLen) return false;
    for (char c : s) {
        if (!isAllowedChar(c)) return false;
    }
    return true;
}

static bool isSafePathToken(const std::string &s) {
    if (!isSafeToken(s)) return false;
    if (s.size() >= 2 && s.find("..") != std::string::npos) return false;
    if (!s.empty() && s[0] == '/') return false; // do not allow absolute paths as args
    return true;
}

static bool splitTokens(const std::string &input, std::vector<std::string> &outTokens, std::string &err) {
    outTokens.clear();
    if (input.empty()) { err = "Empty input."; return false; }
    if (input.size() > 256) { err = "Input too long."; return false; }
    std::istringstream iss(input);
    std::string tok;
    while (iss >> tok) {
        outTokens.push_back(tok);
        if (outTokens.size() > 10) { err = "Too many arguments (max 10)."; return false; }
    }
    if (outTokens.empty()) { err = "No tokens found."; return false; }
    return true;
}

static std::string basenameOf(const std::string &path) {
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) return path;
    if (pos + 1 >= path.size()) return "";
    return path.substr(pos + 1);
}

static bool validateAndBuildArgv(const std::string &input, std::vector<std::string> &argvOut, std::string &err) {
    argvOut.clear();
    std::vector<std::string> tokens;
    if (!splitTokens(input, tokens, err)) return false;

    std::unordered_map<std::string, std::string> cmdPaths = {
        {"echo", "/bin/echo"},
        {"ls", "/bin/ls"},
        {"uname", "/bin/uname"},
        {"id", "/usr/bin/id"},
        {"date", "/bin/date"},
        {"whoami", "/usr/bin/whoami"}
    };

    std::string cmdToken = tokens[0];
    std::string base = basenameOf(cmdToken);
    if (cmdPaths.find(base) == cmdPaths.end()) {
        err = "Command not allowed.";
        return false;
    }
    const std::string execPath = cmdPaths[base];

    // Validate all tokens (basic safe charset)
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (!isSafeToken(tokens[i], i == 0 ? 64 : 128)) {
            err = "Token contains disallowed characters or is too long.";
            return false;
        }
    }

    // Per-command stricter validation
    argvOut.push_back(execPath);
    if (base == "echo") {
        // Allow any number of safe tokens as args
        for (size_t i = 1; i < tokens.size(); ++i) argvOut.push_back(tokens[i]);
    } else if (base == "ls") {
        static const std::unordered_set<std::string> allowedOpts = {"-l","-a","-al","-la"};
        for (size_t i = 1; i < tokens.size(); ++i) {
            const std::string &t = tokens[i];
            if (!t.empty() && t[0] == '-') {
                if (allowedOpts.find(t) == allowedOpts.end()) { err = "Unsupported ls option."; return false; }
                argvOut.push_back(t);
            } else {
                if (!isSafePathToken(t)) { err = "Unsafe ls path argument."; return false; }
                argvOut.push_back(t);
            }
        }
    } else if (base == "uname") {
        static const std::unordered_set<std::string> allowedOpts = {"-a","-s","-r","-m","-n","-v","-p","-o"};
        for (size_t i = 1; i < tokens.size(); ++i) {
            const std::string &t = tokens[i];
            if (t.empty() || t[0] != '-') { err = "uname only accepts options."; return false; }
            if (allowedOpts.find(t) == allowedOpts.end()) { err = "Unsupported uname option."; return false; }
            argvOut.push_back(t);
        }
    } else if (base == "id") {
        static const std::unordered_set<std::string> allowedOpts = {"-u","-g"};
        for (size_t i = 1; i < tokens.size(); ++i) {
            const std::string &t = tokens[i];
            if (t.empty() || t[0] != '-') { err = "id only accepts options."; return false; }
            if (allowedOpts.find(t) == allowedOpts.end()) { err = "Unsupported id option."; return false; }
            argvOut.push_back(t);
        }
    } else if (base == "date") {
        for (size_t i = 1; i < tokens.size(); ++i) {
            const std::string &t = tokens[i];
            if (!t.empty() && t[0] == '-') {
                if (t != "-u") { err = "Unsupported date option."; return false; }
                argvOut.push_back(t);
            } else if (!t.empty() && t[0] == '+') {
                if (t.size() > 64) { err = "date format too long."; return false; }
                // Check allowed chars for format string after '+'
                bool ok = true;
                for (size_t k = 1; k < t.size(); ++k) {
                    char c = t[k];
                    if (!(std::isalnum(static_cast<unsigned char>(c)) ||
                          c == '%' || c == ':' || c == '.' || c == '/' || c == '_' || c == '-')) {
                        ok = false; break;
                    }
                }
                if (!ok) { err = "date format contains disallowed characters."; return false; }
                argvOut.push_back(t);
            } else {
                err = "Unsupported date argument.";
                return false;
            }
        }
    } else if (base == "whoami") {
        if (tokens.size() != 1) { err = "whoami does not accept arguments."; return false; }
    } else {
        err = "Internal error.";
        return false;
    }

    return true;
}

static bool setCloExec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) return false;
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) return false;
    return true;
}

static std::string readAllFromFdLimited(int fd, size_t maxBytes) {
    std::string out;
    out.reserve(std::min<size_t>(4096, maxBytes));
    char buf[4096];
    size_t total = 0;
    while (true) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            size_t toCopy = static_cast<size_t>(n);
            if (total + toCopy > maxBytes) {
                toCopy = maxBytes - total;
            }
            out.append(buf, buf + toCopy);
            total += toCopy;
            if (total >= maxBytes) break;
        } else if (n == 0) {
            break;
        } else {
            if (errno == EINTR) continue;
            break;
        }
    }
    return out;
}

static ExecResult runCommandArgv(const std::vector<std::string> &argvIn) {
    ExecResult res;
    res.exitCode = -1;

    int outPipe[2] = {-1, -1};
    int errPipe[2] = {-1, -1};
    if (pipe(outPipe) == -1 || pipe(errPipe) == -1) {
        res.error = "Failed to create pipes.";
        if (outPipe[0] != -1) { close(outPipe[0]); close(outPipe[1]); }
        if (errPipe[0] != -1) { close(errPipe[0]); close(errPipe[1]); }
        return res;
    }
    setCloExec(outPipe[0]); setCloExec(outPipe[1]);
    setCloExec(errPipe[0]); setCloExec(errPipe[1]);

    pid_t pid = fork();
    if (pid == -1) {
        res.error = "fork failed.";
        close(outPipe[0]); close(outPipe[1]);
        close(errPipe[0]); close(errPipe[1]);
        return res;
    }
    if (pid == 0) {
        // Child
        // Redirect stdout and stderr
        (void)dup2(outPipe[1], STDOUT_FILENO);
        (void)dup2(errPipe[1], STDERR_FILENO);

        // Close all pipe fds
        close(outPipe[0]); close(outPipe[1]);
        close(errPipe[0]); close(errPipe[1]);

        // Build argv
        std::vector<char*> argv;
        argv.reserve(argvIn.size() + 1);
        for (const auto &s : argvIn) {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
        argv.push_back(nullptr);

        // Exec with absolute path (argvIn[0])
        execv(argvIn[0].c_str(), argv.data());
        // If exec fails
        const char msg[] = "exec failed\n";
        write(STDERR_FILENO, msg, sizeof(msg)-1);
        _exit(127);
    }

    // Parent
    close(outPipe[1]);
    close(errPipe[1]);

    res.stdoutStr = readAllFromFdLimited(outPipe[0], 65536);
    res.stderrStr = readAllFromFdLimited(errPipe[0], 65536);

    close(outPipe[0]);
    close(errPipe[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        res.error = "waitpid failed.";
        res.exitCode = -1;
        return res;
    }

    if (WIFEXITED(status)) {
        res.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        res.exitCode = 128 + WTERMSIG(status);
    } else {
        res.exitCode = -1;
    }
    return res;
}

static std::string execute_validated_command(const std::string &input) {
    std::vector<std::string> argv;
    std::string err;
    if (!validateAndBuildArgv(input, argv, err)) {
        std::ostringstream oss;
        oss << "error: " << err << "\n";
        return oss.str();
    }
    ExecResult r = runCommandArgv(argv);
    std::ostringstream oss;
    if (!r.error.empty()) {
        oss << "error: " << r.error << "\n";
        return oss.str();
    }
    oss << "exit=" << r.exitCode << "\n";
    oss << "stdout:\n" << r.stdoutStr;
    if (!r.stdoutStr.empty() && r.stdoutStr.back() != '\n') oss << "\n";
    oss << "stderr:\n" << r.stderrStr;
    if (!r.stderrStr.empty() && r.stderrStr.back() != '\n') oss << "\n";
    return oss.str();
}

int main() {
    // 5 test cases
    std::vector<std::string> tests = {
        "echo Hello_World",
        "ls -l",
        "uname -a",
        "id -u",
        "date +%Y-%m-%d"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i+1) << ": " << tests[i] << "\n";
        std::cout << execute_validated_command(tests[i]) << "\n";
    }

    // Read a command from user input
    std::cout << "Enter a command: ";
    std::string line;
    if (std::getline(std::cin, line)) {
        std::cout << execute_validated_command(line);
    }
    return 0;
}