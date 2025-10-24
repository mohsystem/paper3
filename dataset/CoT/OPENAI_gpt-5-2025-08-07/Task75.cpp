#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <processthreadsapi.h>
#else
  #include <signal.h>
  #include <errno.h>
  #include <unistd.h>
  #include <time.h>
#endif

static bool parse_ll(const char* s, long long& out) {
    if (!s) return false;
    char* end = nullptr;
    errno = 0;
    long long v = strtoll(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return false;
    out = v;
    return true;
}

#ifdef _WIN32
static bool terminate_process(long long pid_ll) {
    if (pid_ll <= 0) return false;
    DWORD pid = (DWORD)pid_ll;

    // Block terminating self
    if (pid == GetCurrentProcessId()) return false;
    // Block likely critical PIDs
    if (pid <= 4) return false;

    HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!h) {
        // Could be access denied or not found
        return false;
    }

    // TerminateProcess is forceful on Windows
    BOOL ok = TerminateProcess(h, 1);
    if (!ok) {
        CloseHandle(h);
        return false;
    }

    DWORD waitRes = WaitForSingleObject(h, 5000);
    CloseHandle(h);
    return (waitRes == WAIT_OBJECT_0);
}
#else
static bool is_alive(pid_t pid) {
    if (pid <= 0) return false;
    int r = kill(pid, 0);
    if (r == 0) return true;
    if (errno == ESRCH) return false;
    return true; // EPERM or others -> assume alive
}

static void sleep_ms(long ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, nullptr);
}

static bool terminate_process(long long pid_ll) {
    if (pid_ll <= 0) return false;
    pid_t pid = (pid_t)pid_ll;

    // Block terminating self
    if (pid == getpid()) return false;
    // Block critical
    if (pid <= 1) return false;

    // If not alive, already terminated
    if (!is_alive(pid)) return true;

    // Try graceful SIGTERM
    if (kill(pid, SIGTERM) != 0) {
        if (errno == ESRCH) return true;
        if (errno == EPERM) return false;
        // Other errors: continue
    }

    // Wait up to 2s
    for (int i = 0; i < 40; ++i) {
        if (!is_alive(pid)) return true;
        sleep_ms(50);
    }

    // Force with SIGKILL
    if (kill(pid, SIGKILL) != 0) {
        if (errno == ESRCH) return true;
        if (errno == EPERM) return false;
    }

    for (int i = 0; i < 60; ++i) {
        if (!is_alive(pid)) return true;
        sleep_ms(50);
    }

    return !is_alive(pid);
}
#endif

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            long long pid;
            if (!parse_ll(argv[i], pid)) {
                std::printf("Arg '%s' is not a valid integer PID\n", argv[i]);
                continue;
            }
            bool ok = terminate_process(pid);
            std::printf("Terminate PID %lld: %s\n", pid, ok ? "SUCCESS/NOT ALIVE" : "FAILED/ALIVE");
        }
    } else {
        // 5 demonstrative test cases
        long long tests[5];
#ifdef _WIN32
        tests[0] = -1;
        tests[1] = 0;
        tests[2] = 4; // critical/system on Windows
        tests[3] = (long long)GetCurrentProcessId(); // self
        tests[4] = 999999;
#else
        tests[0] = -1;
        tests[1] = 0;
        tests[2] = 1; // init/system on POSIX
        tests[3] = (long long)getpid(); // self
        tests[4] = 999999;
#endif
        std::puts("No arguments provided. Running 5 demonstrative test cases.");
        for (int i = 0; i < 5; ++i) {
            bool ok = terminate_process(tests[i]);
            std::printf("Terminate PID %lld: %s\n", tests[i], ok ? "SUCCESS/NOT ALIVE" : "FAILED/ALIVE");
        }
    }
    return 0;
}