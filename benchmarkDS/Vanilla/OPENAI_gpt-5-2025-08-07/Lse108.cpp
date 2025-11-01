#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

static std::string shell_quote(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 4 + 3);
    out.push_back('\'');
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

int getent_to_file(const std::string& username, FILE* output) {
    if (!output) return -1;
    fflush(output);
    int out_fd = fileno(output);
    if (out_fd < 0) return -1;

    int saved = dup(STDOUT_FILENO);
    if (saved < 0) return -1;

    int rc = -1;
    if (dup2(out_fd, STDOUT_FILENO) >= 0) {
        std::string cmd = "getent passwd " + shell_quote(username);
        rc = std::system(cmd.c_str());
        fflush(stdout);
        dup2(saved, STDOUT_FILENO);
    }
    close(saved);
    return rc;
}

int main() {
    const char* users[5] = {"root", "nobody", "daemon", nullptr, "no_such_user_xyz_12345"};

    const char* env_user = std::getenv("USER");
    if (env_user && std::strlen(env_user) > 0) {
        users[3] = env_user;
    } else {
        users[3] = "root";
    }

    const char* files[5] = {"cpp_out1.txt", "cpp_out2.txt", "cpp_out3.txt", "cpp_out4.txt", "cpp_out5.txt"};

    for (int i = 0; i < 5; ++i) {
        FILE* f = std::fopen(files[i], "w");
        if (f) {
            getent_to_file(users[i], f);
            std::fclose(f);
        }
    }
    return 0;
}