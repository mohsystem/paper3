#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cctype>

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#endif

static bool is_safe_char(char c) {
    if (std::isalnum(static_cast<unsigned char>(c))) return true;
    const std::string allowed = " _.,:@%+/=-/"; // '-' and '/' included
    return allowed.find(c) != std::string::npos;
}

static bool is_safe(const std::string& s) {
    if (s.size() > 200) return false;
    for (char c : s) {
        if (!is_safe_char(c)) return false;
    }
    return true;
}

std::string runEcho(const std::string& input) {
    if (!is_safe(input)) {
        throw std::runtime_error("Input contains disallowed characters or is too long.");
    }

#if defined(_WIN32)
    // Use cmd.exe /c echo with sanitized input
    std::string cmd = "cmd.exe /c echo " + input;
    // Use _popen to capture output (invokes shell - acceptable due to strict sanitization)
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("Failed to start process.");
    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output += buffer;
    }
    _pclose(pipe);
    // Trim trailing CR/LF
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) output.pop_back();
    return output;
#else
    int fd[2];
    if (pipe(fd) == -1) {
        throw std::runtime_error("pipe failed");
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(fd[0]); close(fd[1]);
        throw std::runtime_error("fork failed");
    }
    if (pid == 0) {
        // Child
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execl("/bin/echo", "echo", input.c_str(), (char*)nullptr);
        _exit(127);
    } else {
        // Parent
        close(fd[1]);
        std::string output;
        char buf[512];
        ssize_t n;
        while ((n = read(fd[0], buf, sizeof(buf))) > 0) {
            output.append(buf, buf + n);
        }
        close(fd[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        // Trim trailing newline(s)
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) output.pop_back();
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw std::runtime_error("Process failed");
        }
        return output;
    }
#endif
}

int main() {
    try {
        // 5 test cases
        std::cout << runEcho("Hello World") << std::endl;
        std::cout << runEcho("Sample_123") << std::endl;
        std::cout << runEcho("Path/with/slash") << std::endl;
        std::cout << runEcho("Value+Plus=Equals") << std::endl;
        std::cout << runEcho("Email: user@example.com") << std::endl;

        // Interactive input
        std::string userInput;
        if (!std::getline(std::cin, userInput)) userInput.clear();
        std::cout << runEcho(userInput) << std::endl;
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}