#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include <signal.h>
  #include <errno.h>
  #include <unistd.h>
#endif

// Return codes:
// 0 = success, 1 = invalid input, 2 = refused to terminate self, 3 = not found or access denied, 4 = system error
int terminate_process(long long pid, int force) {
#ifdef _WIN32
    const unsigned long long MAX_PID = 0xFFFFFFFFULL; /* DWORD range */
    if (pid <= 0 || (unsigned long long)pid > MAX_PID) {
        return 1;
    }
    DWORD self = GetCurrentProcessId();
    if ((unsigned long long)pid == (unsigned long long)self) {
        return 2;
    }
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (h == NULL) {
        return 3;
    }
    (void)force; /* TerminateProcess is always forceful; ignore 'force' */
    BOOL ok = TerminateProcess(h, 1);
    DWORD lastErr = ok ? ERROR_SUCCESS : GetLastError();
    CloseHandle(h);
    if (ok) return 0;
    if (lastErr == ERROR_ACCESS_DENIED || lastErr == ERROR_INVALID_PARAMETER || lastErr == ERROR_NOT_FOUND) return 3;
    return 4;
#else
    if (pid <= 0 || pid > (long long)INT_MAX) {
        return 1;
    }
    pid_t self = getpid();
    if ((pid_t)pid == self) {
        return 2;
    }
    int sig = force ? SIGKILL : SIGTERM;
    if (kill((pid_t)pid, sig) == 0) {
        return 0;
    }
    if (errno == ESRCH || errno == EPERM) return 3;
    return 4;
#endif
}

static int is_force_flag(const char* s) {
    if (s == NULL) return 0;
    if (strcmp(s, "-f") == 0) return 1;
    if (strcmp(s, "--force") == 0) return 1;
    return 0;
}

static int parse_pid(const char* s, long long* out) {
    if (s == NULL || out == NULL) return 0;
    char *end = NULL;
    errno = 0;
    long long v = strtoll(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    *out = v;
    return 1;
}

static void print_result(long long pid, int force, int code) {
    printf("Attempt terminate pid=%lld force=%s -> result=%d\n", pid, force ? "true" : "false", code);
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        long long pid = 0;
        if (!parse_pid(argv[1], &pid)) {
            printf("Invalid PID input. Usage: ./task <pid> [--force|-f]\n");
            return 1;
        }
        int force = (argc >= 3) ? is_force_flag(argv[2]) : 0;
        int code = terminate_process(pid, force);
        print_result(pid, force, code);
        return (code == 0) ? 0 : 1;
    } else {
        /* 5 Test cases */
#ifdef _WIN32
        long long self = (long long)GetCurrentProcessId();
#else
        long long self = (long long)getpid();
#endif
        long long testPids[5] = { -5LL, 0LL, self, 999999999LL, 1LL };
        int forces[5] = { 0, 1, 0, 1, 0 };
        for (int i = 0; i < 5; ++i) {
            int code = terminate_process(testPids[i], forces[i]);
            print_result(testPids[i], forces[i], code);
        }
        return 0;
    }
}