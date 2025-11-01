#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <cerrno>

static const int INVALID_INPUT_STATUS = 2;
static const int TIMEOUT_STATUS = 124;

static bool is_valid_username(const std::string& username) {
    if (username.empty() || username.size() > 128) return false;
    static const std::regex re(R"(^[a-z_][a-z0-9_-]*[$]?$)");
    return std::regex_match(username, re);
}

int run_id(const std::string& username) {
    if (!is_valid_username(username)) {
        return INVALID_INPUT_STATUS;
    }

    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
        // Child: try /usr/bin/id then /bin/id
        const char* argv1[] = {"/usr/bin/id", username.c_str(), nullptr};
        execv("/usr/bin/id", (char* const*)argv1);
        if (errno == ENOENT) {
            const char* argv2[] = {"/bin/id", username.c_str(), nullptr};
            execv("/bin/id", (char* const*)argv2);
        }
        _exit(127);
    } else {
        // Parent: wait with timeout
        int status = 0;
        const int timeout_ms = 5000;
        int waited = 0;
        while (waited < timeout_ms) {
            pid_t r = waitpid(pid, &status, WNOHANG);
            if (r == pid) {
                if (WIFEXITED(status)) return WEXITSTATUS(status);
                if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
                return 1;
            } else if (r == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                waited += 10;
                continue;
            } else {
                // waitpid error
                return 1;
            }
        }
        // Timeout
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        return TIMEOUT_STATUS;
    }
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        std::string user = argv[1];
        int status = run_id(user);
        std::cout << "Exit status for username '" << user << "': " << status << "\n";
    } else {
        std::vector<std::string> tests = {"root", "nobody", "daemon", "syslog", "nonexistentuser12345"};
        for (const auto& u : tests) {
            int status = run_id(u);
            std::cout << "Exit status for username '" << u << "': " << status << "\n";
        }
    }
    return 0;
}