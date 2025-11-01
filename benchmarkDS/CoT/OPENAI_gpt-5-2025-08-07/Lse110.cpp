// Chain-of-Through process in code generation:
// 1) Problem understanding: Provide a function that takes a username, runs '/usr/bin/id -- <username>', returns exit status, and print it.
// 2) Security requirements: Avoid shell, use absolute path, prevent option injection by '--', handle signals and errors.
// 3) Secure coding generation: Use fork/execv with argument array, waitpid to get exit status.
// 4) Code review: No shell usage, absolute path, '--' separator, proper waitpid checks and status decoding.
// 5) Secure code output: Final code with error handling and 5 test cases.

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

int run_id(const std::string& username) {
    if (username.empty()) {
        return 2; // invalid input
    }
    pid_t pid = fork();
    if (pid < 0) {
        return 1; // fork failed
    }
    if (pid == 0) {
        // Child: execute /usr/bin/id -- username
        execl("/usr/bin/id", "id", "--", username.c_str(), (char*)nullptr);
        _exit(127); // exec failed
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return 1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        int status = run_id(std::string(argv[1]));
        std::cout << "Exit status: " << status << std::endl;
        return 0;
    }
    // 5 test cases
    std::vector<std::string> tests = {
        "root",
        "nobody",
        "daemon",
        "www-data",
        "user_does_not_exist_123456"
    };
    for (const auto& u : tests) {
        int status = run_id(u);
        std::cout << "Username: " << u << " -> Exit status: " << status << std::endl;
    }
    return 0;
}