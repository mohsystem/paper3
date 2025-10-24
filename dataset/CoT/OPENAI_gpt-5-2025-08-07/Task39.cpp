// Task39 - C++
// Chain-of-Through in comments:
// 1) Implement function that uses a system command (cat) to display file content.
// 2) Security: validate filename via whitelist; ensure it is a regular file; avoid invoking a shell; pass "--" to cat.
// 3) Secure coding: use fork/execv and pipes on POSIX; no shell; sanitize input.
// 4) Review: check lengths/chars; handle errors; ensure resources closed.
// 5) Output: final code with 5 test cases.
//
// Note: This C++ implementation targets POSIX systems.

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <cstdlib>

static const size_t MAX_FILENAME_LEN_CPP = 4096;

static bool is_regular_file_cpp(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

static bool is_safe_filename_cpp(const std::string& filename) {
    if (filename.empty() || filename.size() > MAX_FILENAME_LEN_CPP) return false;
    if (filename.find('\0') != std::string::npos) return false;
    static const std::regex safe("^[A-Za-z0-9_./\\\\ \\-:]+$");
    if (!std::regex_match(filename, safe)) return false;
    return true;
}

std::string display_file_secure(const std::string& filename) {
    if (!is_safe_filename_cpp(filename)) {
        return "Error: Invalid filename.";
    }
    if (!is_regular_file_cpp(filename)) {
        return "Error: File not found or not a regular file.";
    }

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return "Error: Failed to create pipe.";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return "Error: Failed to fork.";
    } else if (pid == 0) {
        // Child: exec /bin/cat -- filename, stdout -> pipe
        // Close read end
        close(pipefd[0]);
        // Redirect stdout
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) _exit(127);
        // Redirect stderr to stdout for simplicity
        if (dup2(pipefd[1], STDERR_FILENO) == -1) _exit(127);
        // Close write end after dup
        close(pipefd[1]);

        const char* prog = "/bin/cat";
        const char* argv0 = "cat";
        const char* dashdash = "--";
        const char* fname = filename.c_str();
        const char* args[] = {argv0, dashdash, fname, nullptr};
        execv(prog, (char* const*)args);
        _exit(127);
    } else {
        // Parent: read from pipe, wait child
        close(pipefd[1]);
        std::string out;
        char buf[4096];
        ssize_t n;
        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
            out.append(buf, buf + n);
        }
        close(pipefd[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return out;
        } else {
            std::ostringstream oss;
            int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            oss << "Error: Command failed with exit code " << code << ". Output:\n" << out;
            return oss.str();
        }
    }
}

static std::string join_path(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

static std::string write_file_in_dir(const std::string& dir, const std::string& name, const std::string& content) {
    std::string path = join_path(dir, name);
    int fd = open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd >= 0) {
        ssize_t off = 0;
        const char* data = content.c_str();
        ssize_t left = (ssize_t)content.size();
        while (left > 0) {
            ssize_t w = write(fd, data + off, (size_t)left);
            if (w <= 0) break;
            off += w;
            left -= w;
        }
        close(fd);
    }
    return path;
}

int main() {
    // Create temporary directory
    char tmpl[] = "/tmp/task39-cpp-XXXXXX";
    char* d = mkdtemp(tmpl);
    if (!d) {
        std::cerr << "Failed to create temp dir\n";
        return 1;
    }
    std::string dir = d;

    std::string f1 = write_file_in_dir(dir, "file1.txt", "Hello from file1\nLine2\n");
    std::string f2 = write_file_in_dir(dir, "file with spaces.txt", "Content with spaces in filename\n");
    std::string f3 = write_file_in_dir(dir, "-leadingDash.txt", "Filename starts with dash\n");
    std::string f4 = write_file_in_dir(dir, "normal-_.txt", "Normal file name content\n");
    std::string f5 = join_path(dir, "does_not_exist.txt");

    std::cout << "Test1:\n" << display_file_secure(f1);
    std::cout << "Test2:\n" << display_file_secure(f2);
    std::cout << "Test3:\n" << display_file_secure(f3);
    std::cout << "Test4:\n" << display_file_secure(f4);
    std::cout << "Test5:\n" << display_file_secure(f5);

    return 0;
}