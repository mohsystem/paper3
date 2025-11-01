#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <fcntl.h>
extern char **environ;
#endif

struct PingResult {
    bool alive;
    std::string pong;
    std::string hostAscii;
};

static std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return s.substr(a, b - a);
}

static bool isSafeHostAscii(const std::string& h) {
    if (h.empty() || h.size() > 253) return false;
    for (char c : h) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '.' || c == ':'))
            return false;
    }
    return true;
}

static std::string extractHostAscii(const std::string& url) {
    std::string u = trim(url);
    if (u.empty()) throw std::invalid_argument("url empty");
    std::string host;
    size_t start = 0;
    size_t scheme = u.find("://");
    if (scheme != std::string::npos) start = scheme + 3;
    size_t end = u.find_first_of("/?#", start);
    if (end == std::string::npos) end = u.size();
    host = u.substr(start, end - start);
    if (host.size() >= 2 && host.front() == '[' && host.back() == ']') {
        host = host.substr(1, host.size() - 2);
    }
    if (!isSafeHostAscii(host)) throw std::invalid_argument("invalid host");
    return host;
}

static PingResult pingUrl(const std::string& url) {
    PingResult res{false, "", ""};
    try {
        res.hostAscii = extractHostAscii(url);
    } catch (const std::exception& e) {
        res.pong = std::string("Error: ") + e.what();
        return res;
    }

#if defined(_WIN32)
    // Build command line: ping -n 1 -w 2000 host
    std::wstring cmd = L"ping -n 1 -w 2000 ";
    // hostAscii is restricted to ASCII safe chars
    std::wstring whost(res.hostAscii.begin(), res.hostAscii.end());
    cmd += whost;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE outRead = NULL, outWrite = NULL;
    if (!CreatePipe(&outRead, &outWrite, &sa, 0)) {
        res.pong = "CreatePipe failed";
        return res;
    }
    if (!SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(outRead); CloseHandle(outWrite);
        res.pong = "SetHandleInformation failed";
        return res;
    }

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = outWrite;
    si.hStdError  = outWrite;
    si.hStdInput  = NULL;

    PROCESS_INFORMATION pi{};
    // CreateProcess requires mutable buffer
    std::vector<wchar_t> buffer(cmd.begin(), cmd.end());
    buffer.push_back(L'\0');
    BOOL ok = CreateProcessW(
        NULL, buffer.data(), NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi
    );
    CloseHandle(outWrite); // child has a copy
    if (!ok) {
        CloseHandle(outRead);
        res.pong = "CreateProcess failed";
        return res;
    }

    DWORD waitRes = WaitForSingleObject(pi.hProcess, 5000);
    if (waitRes == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        res.pong += "[Timeout]\n";
    }

    // Read output
    char buf[4096];
    DWORD n;
    size_t limit = 65536;
    while (ReadFile(outRead, buf, sizeof(buf), &n, NULL) && n > 0) {
        if (res.pong.size() + n > limit) {
            res.pong.append(buf, buf + (limit - res.pong.size()));
            break;
        }
        res.pong.append(buf, buf + n);
    }
    CloseHandle(outRead);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    res.alive = (exitCode == 0);
    return res;

#else
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        res.pong = "pipe failed";
        return res;
    }
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);

    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("ping"));
#if defined(__APPLE__)
    argv.push_back(const_cast<char*>("-c"));
    argv.push_back(const_cast<char*>("1"));
    argv.push_back(const_cast<char*>("-W"));
    argv.push_back(const_cast<char*>("2000"));
#else
    argv.push_back(const_cast<char*>("-c"));
    argv.push_back(const_cast<char*>("1"));
    argv.push_back(const_cast<char*>("-W"));
    argv.push_back(const_cast<char*>("2"));
#endif
    argv.push_back(const_cast<char*>(res.hostAscii.c_str()));
    argv.push_back(nullptr);

    pid_t pid;
    int spawnRes = posix_spawnp(&pid, "ping", &actions, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]); // parent reads

    if (spawnRes != 0) {
        close(pipefd[0]);
        res.pong = "posix_spawnp failed";
        return res;
    }

    // Read output
    char buf[4096];
    ssize_t n;
    size_t limit = 65536;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (res.pong.size() + static_cast<size_t>(n) > limit) {
            size_t toCopy = limit - res.pong.size();
            res.pong.append(buf, buf + toCopy);
            break;
        }
        res.pong.append(buf, buf + n);
    }
    close(pipefd[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    res.alive = WIFEXITED(status) && (WEXITSTATUS(status) == 0);
    return res;
#endif
}

int main() {
    std::vector<std::string> tests = {
        "https://example.com",
        "example.com",
        "8.8.8.8",
        "http://[::1]",
        "bad host!!"
    };
    for (const auto& t : tests) {
        PingResult r = pingUrl(t);
        std::cout << "Test: " << t
                  << " -> alive=" << (r.alive ? "true" : "false")
                  << " host=" << r.hostAscii
                  << " pongLen=" << r.pong.size() << "\n";
    }
    return 0;
}