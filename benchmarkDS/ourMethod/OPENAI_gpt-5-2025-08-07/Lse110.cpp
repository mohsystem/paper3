#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// Validate username: length 1..32, ASCII only, starts with [A-Za-z_],
// then [A-Za-z0-9_.-], optional trailing '$'
static bool is_valid_username(const std::string& u) {
    if (u.empty() || u.size() > 32) return false;
    for (unsigned char ch : u) {
        if (ch & 0x80) return false; // non-ASCII
    }
    bool trailing_dollar = (!u.empty() && u.back() == '$');
    std::string base = trailing_dollar ? u.substr(0, u.size() - 1) : u;
    if (base.empty()) return false;

    unsigned char first = static_cast<unsigned char>(base[0]);
    if (!(std::isalpha(first) || first == '_')) return false;

    for (size_t i = 1; i < base.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(base[i]);
        if (!(std::isalnum(c) || c == '_' || c == '-' || c == '.')) return false;
    }
    return true;
}

int run_id_command(const std::string& username) {
    if (!is_valid_username(username)) {
        std::cerr << "Invalid username.\n";
        return 2; // invalid input
    }
    pid_t pid = fork();
    if (pid < 0) {
        return 127; // fork failed
    } else if (pid == 0) {
        // Child: execute id <username>
        execlp("id", "id", username.c_str(), (char*)NULL);
        _exit(127); // exec failed
    } else {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            return 127;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        } else {
            return 127;
        }
    }
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        std::string username = argv[1];
        int rc = run_id_command(username);
        std::cout << "Exit status: " << rc << "\n";
        return 0;
    }
    // 5 test cases
    std::vector<std::string> tests = {"root", "daemon", "nobody", "bin", "this_user_should_not_exist_zzz"};
    for (const auto& t : tests) {
        std::cout << "Running: id " << t << "\n";
        int rc = run_id_command(t);
        std::cout << "Exit status: " << rc << "\n";
    }
    return 0;
}