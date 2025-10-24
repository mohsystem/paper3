#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include <csignal>
  #include <cerrno>
  #include <unistd.h>
#endif

// Return codes:
// 0 = success, 1 = invalid input, 2 = refused to terminate self, 3 = not found or access denied, 4 = system error
int terminate_process(long long pid, bool force) {
    // Validate range
#ifdef _WIN32
    const unsigned long long MAX_PID = 0xFFFFFFFFULL; // DWORD range
#else
    const long long MAX_PID = static_cast<long long>(std::numeric_limits<int>::max());
#endif
    if (pid <= 0 || pid > MAX_PID) {
        return 1;
    }

    // Prevent terminating self
#ifdef _WIN32
    DWORD self = GetCurrentProcessId();
    if ((unsigned long long)pid == (unsigned long long)self) {
        return 2;
    }
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (h == NULL) {
        return 3;
    }
    // On Windows, TerminateProcess is always forceful; ignore 'force'
    BOOL ok = TerminateProcess(h, 1);
    DWORD lastErr = ok ? ERROR_SUCCESS : GetLastError();
    CloseHandle(h);
    if (ok) return 0;
    // Access denied or process already exited
    if (lastErr == ERROR_ACCESS_DENIED || lastErr == ERROR_INVALID_PARAMETER || lastErr == ERROR_NOT_FOUND) return 3;
    return 4;
#else
    pid_t self = getpid();
    if ((pid_t)pid == self) {
        return 2;
    }
    int sig = force ? SIGKILL : SIGTERM;
    if (kill((pid_t)pid, sig) == 0) {
        return 0;
    }
    // Map common errno values
    if (errno == ESRCH || errno == EPERM) return 3;
    return 4;
#endif
}

static bool is_force_flag(const std::string &s) {
    return (s == "-f" || s == "--force");
}

static bool parse_pid(const std::string &s, long long &out) {
    try {
        size_t idx = 0;
        long long v = std::stoll(s, &idx, 10);
        if (idx != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

static void print_result(long long pid, bool force, int code) {
    std::cout << "Attempt terminate pid=" << pid << " force=" << (force ? "true" : "false")
              << " -> result=" << code << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        long long pid = 0;
        if (!parse_pid(argv[1], pid)) {
            std::cout << "Invalid PID input. Usage: ./task <pid> [--force|-f]" << std::endl;
            return 1;
        }
        bool force = (argc >= 3) && is_force_flag(argv[2]);
        int code = terminate_process(pid, force);
        print_result(pid, force, code);
        return (code == 0) ? 0 : 1;
    } else {
        // 5 Test cases
#ifdef _WIN32
        long long self = (long long)GetCurrentProcessId();
#else
        long long self = (long long)getpid();
#endif
        long long testPids[5] = { -5LL, 0LL, self, 999999999LL, 1LL };
        bool forces[5] = { false, true, false, true, false };
        for (int i = 0; i < 5; ++i) {
            int code = terminate_process(testPids[i], forces[i]);
            print_result(testPids[i], forces[i], code);
        }
        return 0;
    }
}