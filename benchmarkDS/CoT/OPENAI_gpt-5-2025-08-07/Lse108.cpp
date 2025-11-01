#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <vector>

// Secure helper: validate username to prevent command injection
static bool is_valid_username(const std::string& u) {
    if (u.empty() || u.size() > 32) return false;
    for (unsigned char ch : u) {
        if (!(std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-')) return false;
    }
    return true;
}

// Core function: executes "getent passwd <username>" via system(), directs output to FILE* output, returns captured content if seekable
static std::string getUserInfoToOutput(const std::string& username, FILE* output) {
    if (!output) return std::string();
    if (!is_valid_username(username)) return std::string();

    int fd = fileno(output);
    if (fd < 0) return std::string();

    // Capture current position if seekable
    long pos_before = -1;
    pos_before = std::ftell(output);

    std::fflush(output);

    // Build command safely; username has been validated to avoid injection
    char cmd[256];
    if (username.size() + 64 >= sizeof(cmd)) return std::string();
    std::snprintf(cmd, sizeof(cmd), "getent passwd %s 1>&%d 2>&%d", username.c_str(), fd, fd);

    int rc = std::system(cmd);
    (void)rc; // even if command fails, we still try to read whatever was written, if any

    std::fflush(output);

    // Try to read back just-written data if the stream is seekable
    if (pos_before >= 0) {
        if (std::fseek(output, 0, SEEK_END) == 0) {
            long end_pos = std::ftell(output);
            if (end_pos >= 0 && end_pos >= pos_before) {
                long sz = end_pos - pos_before;
                if (std::fseek(output, pos_before, SEEK_SET) == 0 && sz >= 0) {
                    std::string buffer;
                    buffer.resize(static_cast<size_t>(sz));
                    size_t readn = std::fread(buffer.data(), 1, static_cast<size_t>(sz), output);
                    buffer.resize(readn);
                    // Restore position to end
                    std::fseek(output, end_pos, SEEK_SET);
                    return buffer;
                }
            }
        }
    }
    return std::string();
}

// Test harness with 5 test cases
int main() {
    std::vector<std::string> tests = {
        "root",
        "nobody",
        "daemon",
        "doesnotexistuser",
        "root;echo HACKED"
    };

    for (const auto& t : tests) {
        FILE* tmp = std::tmpfile();
        if (!tmp) {
            std::perror("tmpfile");
            return 1;
        }
        std::string out = getUserInfoToOutput(t, tmp);
        std::cout << "Test username='" << t << "' -> Output: " << out << std::endl;
        std::fclose(tmp);
    }
    return 0;
}