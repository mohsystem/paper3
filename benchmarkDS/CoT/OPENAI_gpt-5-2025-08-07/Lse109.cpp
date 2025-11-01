#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <iostream>

static bool validate_username(const std::string& username) {
    if (username.empty() || username.size() > 64) return false;
    for (char c : username) {
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

// Execute: cat /etc/passwd | grep -F -- username; return output as string
std::string get_user_info(const std::string& username) {
    if (!validate_username(username)) {
        return std::string();
    }

    int pipe_cat_grep[2];
    int pipe_grep_parent[2];
    if (pipe(pipe_cat_grep) == -1) return std::string();
    if (pipe(pipe_grep_parent) == -1) {
        close(pipe_cat_grep[0]); close(pipe_cat_grep[1]);
        return std::string();
    }

    pid_t pid_cat = fork();
    if (pid_cat == -1) {
        close(pipe_cat_grep[0]); close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]); close(pipe_grep_parent[1]);
        return std::string();
    }

    if (pid_cat == 0) {
        // cat child
        dup2(pipe_cat_grep[1], STDOUT_FILENO);
        close(pipe_cat_grep[0]);
        close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]);
        close(pipe_grep_parent[1]);
        execlp("cat", "cat", "/etc/passwd", (char*)NULL);
        _exit(127);
    }

    pid_t pid_grep = fork();
    if (pid_grep == -1) {
        close(pipe_cat_grep[0]); close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]); close(pipe_grep_parent[1]);
        waitpid(pid_cat, nullptr, 0);
        return std::string();
    }

    if (pid_grep == 0) {
        // grep child
        dup2(pipe_cat_grep[0], STDIN_FILENO);
        dup2(pipe_grep_parent[1], STDOUT_FILENO);
        close(pipe_cat_grep[0]);
        close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]);
        close(pipe_grep_parent[1]);
        execlp("grep", "grep", "-F", "--", username.c_str(), (char*)NULL);
        _exit(127);
    }

    // Parent
    close(pipe_cat_grep[0]);
    close(pipe_cat_grep[1]);
    close(pipe_grep_parent[1]);

    std::string output;
    char buf[8192];
    ssize_t r;
    while ((r = read(pipe_grep_parent[0], buf, sizeof(buf))) > 0) {
        output.append(buf, buf + r);
    }
    close(pipe_grep_parent[0]);

    // Wait for children
    waitpid(pid_cat, nullptr, 0);
    waitpid(pid_grep, nullptr, 0);

    return output;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string res = get_user_info(argv[1]);
        std::cout << res;
    } else {
        // 5 test cases
        std::vector<std::string> tests = {"root", "daemon", "nobody", "bin", "thisuserdoesnotexist123"};
        for (const auto& u : tests) {
            std::cout << get_user_info(u);
        }
    }
    return 0;
}