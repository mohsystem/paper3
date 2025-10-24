#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
#endif

namespace fs = std::filesystem;

static bool is_regular_and_readable(const fs::path& p) {
    std::error_code ec;
    if (!fs::exists(p, ec) || ec) return false;
    if (!fs::is_regular_file(p, ec) || ec) return false;
    // Basic readability check
    std::ifstream ifs(p, std::ios::binary);
    return static_cast<bool>(ifs);
}

#ifdef _WIN32
static std::string quoteWindowsArg(const std::string& s) {
    // Quote argument for Windows command line (no shell).
    // Surround with double quotes, escape internal quotes by backslash.
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else out += c;
    }
    out += "\"";
    return out;
}
#endif

std::string displayFileSecure(const std::string& filename) {
    if (filename.empty() || filename.find('\0') != std::string::npos) {
        return "Error: Invalid filename.";
    }

    std::error_code ec;
    fs::path p = fs::absolute(fs::path(filename)).lexically_normal();
    if (!is_regular_and_readable(p)) {
        if (!fs::exists(p, ec)) return "Error: File does not exist.";
        if (!fs::is_regular_file(p, ec)) return "Error: Not a regular file.";
        return "Error: File is not readable.";
    }

#ifdef _WIN32
    HANDLE hRead = NULL, hWrite = NULL;
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return "Error: CreatePipe failed.";
    }

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi{};
    std::string cmd = std::string("more ") + quoteWindowsArg(p.string());

    // CreateProcessA may modify the command line buffer; make a mutable copy.
    std::vector<char> cmdline(cmd.begin(), cmd.end());
    cmdline.push_back('\0');

    BOOL ok = CreateProcessA(
        NULL,
        cmdline.data(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    // Parent doesn't need write end
    CloseHandle(hWrite);

    if (!ok) {
        CloseHandle(hRead);
        return "Error: CreateProcess failed.";
    }

    std::string output;
    char buffer[8192];
    DWORD bytesRead = 0;
    while (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        output.append(buffer, buffer + bytesRead);
    }
    CloseHandle(hRead);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (exitCode != 0) {
        return "Error: Command failed with exit code " + std::to_string(exitCode) + ". Output: " + output;
    }
    return output;
#else
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Error: pipe failed.";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "Error: fork failed.";
    }

    if (pid == 0) {
        // Child
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("cat", "cat", p.c_str(), (char*)NULL);
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    std::string output;
    char buf[8192];
    ssize_t r;
    while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
        output.append(buf, buf + r);
    }
    close(pipefd[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        return "Error: Command failed with exit code " + std::to_string(code) + ". Output: " + output;
    }
    return output;
#endif
}

int main() {
    // Prepare 5 test cases
    fs::path tempDir = fs::temp_directory_path() / "task39_cpp_tests";
    std::error_code ec;
    fs::create_directories(tempDir, ec);

    fs::path f1 = tempDir / "file1.txt";
    fs::path f2 = tempDir / "file with spaces.txt";
    fs::path f3 = tempDir / "empty.txt";
    fs::path invalid = tempDir / "no_such_file.txt";
    fs::path dir = tempDir / "subdir";

    {
        std::ofstream o(f1, std::ios::binary);
        o << "Hello from file1\nLine 2\n";
    }
    {
        std::ofstream o(f2, std::ios::binary);
        o << "Content with spaces in filename\n";
    }
    {
        std::ofstream o(f3, std::ios::binary);
        // empty
    }
    fs::create_directories(dir, ec);

    std::cout << "Test 1 (regular file):\n";
    std::cout << displayFileSecure(f1.string()) << "\n";

    std::cout << "Test 2 (file with spaces):\n";
    std::cout << displayFileSecure(f2.string()) << "\n";

    std::cout << "Test 3 (empty file):\n";
    std::cout << displayFileSecure(f3.string()) << "\n";

    std::cout << "Test 4 (invalid path):\n";
    std::cout << displayFileSecure(invalid.string()) << "\n";

    std::cout << "Test 5 (directory path):\n";
    std::cout << displayFileSecure(dir.string()) << "\n";

    return 0;
}