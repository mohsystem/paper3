#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <string>

int run_id_command(const std::string& username) {
    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
        execlp("id", "id", username.c_str(), (char*)NULL);
        _exit(127);
    } else {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) return 1;
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int status = run_id_command(argv[1]);
        std::cout << status << std::endl;
    } else {
        std::vector<std::string> tests = {"root", "daemon", "nobody", "bin", "this_user_does_not_exist_12345"};
        for (const auto& u : tests) {
            int status = run_id_command(u);
            std::cout << "Exit status for '" << u << "': " << status << std::endl;
        }
    }
    return 0;
}