#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <signal.h>
  #include <errno.h>
  #include <unistd.h>
  #include <time.h>
#endif

static bool isNumber(const std::string& s, long long& out) {
    if (s.empty()) return false;
    char* end = nullptr;
    errno = 0;
    long long val = std::strtoll(s.c_str(), &end, 10);
    if (errno != 0 || end == s.c_str() || *end != '\0') return false;
    out = val;
    return true;
}

#ifndef _WIN32
static bool isAlivePosix(pid_t pid) {
    if (pid <= 0) return false;
    if (kill(pid, 0) == 0) return true;
    if (errno == ESRCH) return false;
    return true;
}
#endif

bool terminateProcess(long long pidInput) {
    if (pidInput <= 1) return false;

#ifdef _WIN32
    DWORD selfPid = GetCurrentProcessId();
    if (pidInput == (long long)selfPid) return false;

    if (pidInput <= 0 || pidInput > std::numeric_limits<DWORD>::max()) return false;
    DWORD pid = static_cast<DWORD>(pidInput);

    HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!h) return false;

    bool success = false;
    if (TerminateProcess(h, 1)) {
        DWORD waitRes = WaitForSingleObject(h, 5000);
        success = (waitRes == WAIT_OBJECT_0);
    }
    CloseHandle(h);
    return success;
#else
    pid_t selfPid = getpid();
    if (pidInput == (long long)selfPid) return false;

    pid_t pid = static_cast<pid_t>(pidInput);

    // Try SIGTERM
    if (kill(pid, SIGTERM) != 0) {
        if (errno == ESRCH) return true; // Already not existent
        return false; // Permission or other error
    }

    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 100000000; // 100ms

    int attempts = 50; // up to 5 seconds
    while (attempts-- > 0) {
        if (!isAlivePosix(pid)) return true;
        nanosleep(&req, nullptr);
    }

    // Escalate to SIGKILL
    if (kill(pid, SIGKILL) != 0) {
        if (errno == ESRCH) return true;
        return false;
    }

    attempts = 50;
    while (attempts-- > 0) {
        if (!isAlivePosix(pid)) return true;
        nanosleep(&req, nullptr);
    }
    return !isAlivePosix(pid);
#endif
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            long long pid;
            if (!isNumber(argv[i], pid)) {
                std::cout << "PID \"" << argv[i] << "\" is invalid input." << std::endl;
                continue;
            }
            bool ok = terminateProcess(pid);
            std::cout << "Terminate PID " << pid << ": " << (ok ? "success" : "failed") << std::endl;
        }
    } else {
        // 5 test cases
#ifdef _WIN32
        long long self = (long long)GetCurrentProcessId();
#else
        long long self = (long long)getpid();
#endif
        std::vector<long long> tests = { -5, 0, 1, self, 99999999LL };
        for (auto pid : tests) {
            bool ok = terminateProcess(pid);
            std::cout << "Test terminate PID " << pid << ": " << (ok ? "success" : "failed") << std::endl;
        }
    }
    return 0;
}