#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>

static bool isValidUsername(const std::string& username) {
    if (username.empty() || username.size() > 32) return false;
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

static void set_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags != -1) {
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    }
}

// Execute a two-command pipeline cmd1 | cmd2, each command is a vector<string>
std::string exec_pipeline(const std::vector<std::vector<std::string>>& commands) {
    if (commands.size() != 2) return "";

    int pipe_cmd[2];   // between cmd1 stdout and cmd2 stdin
    int pipe_out[2];   // cmd2 stdout to parent
    if (pipe(pipe_cmd) == -1) return "";
    if (pipe(pipe_out) == -1) { close(pipe_cmd[0]); close(pipe_cmd[1]); return ""; }

    set_cloexec(pipe_cmd[0]); set_cloexec(pipe_cmd[1]);
    set_cloexec(pipe_out[0]); set_cloexec(pipe_out[1]);

    pid_t c1 = fork();
    if (c1 == -1) {
        close(pipe_cmd[0]); close(pipe_cmd[1]); close(pipe_out[0]); close(pipe_out[1]);
        return "";
    }
    if (c1 == 0) {
        // child 1: cmd1, stdout -> pipe_cmd[1]
        dup2(pipe_cmd[1], STDOUT_FILENO);
        close(pipe_cmd[0]); close(pipe_cmd[1]);
        close(pipe_out[0]); close(pipe_out[1]);

        // Build argv
        std::vector<char*> argv;
        for (const auto& s : commands[0]) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        execv(argv[0], argv.data());
        _exit(127);
    }

    pid_t c2 = fork();
    if (c2 == -1) {
        close(pipe_cmd[0]); close(pipe_cmd[1]); close(pipe_out[0]); close(pipe_out[1]);
        waitpid(c1, nullptr, 0);
        return "";
    }
    if (c2 == 0) {
        // child 2: cmd2, stdin <- pipe_cmd[0], stdout -> pipe_out[1]
        dup2(pipe_cmd[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_cmd[0]); close(pipe_cmd[1]);
        close(pipe_out[0]); close(pipe_out[1]);

        std::vector<char*> argv;
        for (const auto& s : commands[1]) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        execv(argv[0], argv.data());
        _exit(127);
    }

    // Parent
    close(pipe_cmd[0]); close(pipe_cmd[1]);
    close(pipe_out[1]);

    std::string output;
    char buf[8192];
    ssize_t r;
    while ((r = read(pipe_out[0], buf, sizeof(buf))) > 0) {
        output.append(buf, buf + r);
    }
    close(pipe_out[0]);

    int status;
    waitpid(c1, &status, 0);
    waitpid(c2, &status, 0);

    return output;
}

std::string get_user_info(const std::string& username) {
    if (!isValidUsername(username)) return "";
    // Use absolute paths to avoid PATH-based hijacking
    std::vector<std::vector<std::string>> cmds = {
        {"/bin/cat", "/etc/passwd"},
        {"/bin/grep", "-F", username}
    };
    return exec_pipeline(cmds);
}

int main(int argc, char* argv[]) {
    std::vector<std::string> tests = {
        (argc > 1 ? std::string(argv[1]) : std::string("root")),
        "daemon",
        "bin",
        "nosuchuser",
        "invalid;rm"
    };
    for (const auto& u : tests) {
        std::cout << "Query: " << u << "\n";
        std::string out = get_user_info(u);
        std::cout << out;
        if (out.empty()) {
            std::cout << "(no output or invalid username)\n";
        }
        std::cout << "-----\n";
    }
    return 0;
}