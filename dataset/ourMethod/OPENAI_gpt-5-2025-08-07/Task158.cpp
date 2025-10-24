#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

static bool is_valid_input(const std::string& s) {
    if (s.empty() || s.size() > 64) return false;
    for (unsigned char ch : s) {
        bool ok = (ch >= 'A' && ch <= 'Z') ||
                  (ch >= 'a' && ch <= 'z') ||
                  (ch >= '0' && ch <= '9') ||
                  ch == ' ' || ch == '_' || ch == '-' || ch == ':' || ch == '.';
        if (!ok) return false;
    }
    return true;
}

static std::string find_printf_path() {
    const char* candidates[] = {"/usr/bin/printf", "/bin/printf"};
    for (const char* c : candidates) {
        if (access(c, X_OK) == 0) return std::string(c);
    }
    return std::string();
}

std::string run_safe_echo(const std::string& input) {
    if (!is_valid_input(input)) {
        return "ERROR: Invalid input. Allowed: 1-64 chars [A-Za-z0-9 _:-.]";
    }

    std::string printfPath = find_printf_path();
    if (printfPath.empty()) {
        return "ERROR: printf not found on this system.";
    }

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return "ERROR: pipe failed.";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "ERROR: fork failed.";
    }

    if (pid == 0) {
        // Child
        // Redirect stdout to pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(127);
        }
        close(pipefd[0]);
        close(pipefd[1]);

        // exec printf "%s\n" input
        const char* prog = printfPath.c_str();
        const char* arg0 = "printf";
        const char* fmt = "%s\n";
        // Cast away const for execv compatibility
        char* const argv[] = { const_cast<char*>(arg0),
                               const_cast<char*>(fmt),
                               const_cast<char*>(input.c_str()),
                               nullptr };
        execv(prog, argv);
        _exit(127);
    }

    // Parent
    close(pipefd[1]);

    std::string output;
    char buf[1024];
    bool finished = false;
    int status = 0;

    // Non-blocking read is not necessary for small outputs; we loop with timeout.
    for (int i = 0; i < 200; ++i) { // ~2 seconds (10ms * 200)
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) {
            finished = true;
            // Read remaining data
            ssize_t r;
            while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
                output.append(buf, buf + r);
            }
            break;
        } else if (w == 0) {
            // Child still running; attempt to drain any available data
            ssize_t r = read(pipefd[0], buf, sizeof(buf));
            if (r > 0) {
                output.append(buf, buf + r);
            }
            usleep(10000); // 10ms
        } else {
            // Error
            close(pipefd[0]);
            return "ERROR: waitpid failed.";
        }
    }

    if (!finished) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        close(pipefd[0]);
        return "ERROR: Command timed out.";
    }

    close(pipefd[0]);
    return output;
}

int main() {
    std::vector<std::string> tests = {
        "HelloWorld",
        "Safe_123",
        "alpha-beta",
        "COLON:TEST",
        "Numbers 0123456789"
    };
    for (const auto& t : tests) {
        std::string out = run_safe_echo(t);
        std::cout << "Input: " << t << "\n";
        std::cout << "Output: " << out;
        if (out.size() && out.back() != '\n') std::cout << "\n";
        std::cout << "-----\n";
    }
    return 0;
}