#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <signal.h>
  #include <errno.h>
  #include <unistd.h>
  #include <time.h>
#endif

static int isNumberLL(const char* s, long long* out) {
    if (s == NULL || *s == '\0') return 0;
    char* end = NULL;
    errno = 0;
    long long val = strtoll(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    *out = val;
    return 1;
}

#ifndef _WIN32
static int isAlivePosix(pid_t pid) {
    if (pid <= 0) return 0;
    if (kill(pid, 0) == 0) return 1;
    if (errno == ESRCH) return 0;
    return 1;
}
#endif

int terminateProcess(long long pidInput) {
    if (pidInput <= 1) return 0;

#ifdef _WIN32
    DWORD selfPid = GetCurrentProcessId();
    if (pidInput == (long long)selfPid) return 0;

    if (pidInput <= 0 || pidInput > (long long)UINT_MAX) return 0;
    DWORD pid = (DWORD)pidInput;

    HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!h) return 0;

    int success = 0;
    if (TerminateProcess(h, 1)) {
        DWORD waitRes = WaitForSingleObject(h, 5000);
        success = (waitRes == WAIT_OBJECT_0);
    }
    CloseHandle(h);
    return success;
#else
    pid_t selfPid = getpid();
    if (pidInput == (long long)selfPid) return 0;

    pid_t pid = (pid_t)pidInput;

    if (kill(pid, SIGTERM) != 0) {
        if (errno == ESRCH) return 1;
        return 0;
    }

    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 100000000; // 100ms

    int attempts = 50; // up to 5 seconds
    while (attempts-- > 0) {
        if (!isAlivePosix(pid)) return 1;
        nanosleep(&req, NULL);
    }

    if (kill(pid, SIGKILL) != 0) {
        if (errno == ESRCH) return 1;
        return 0;
    }

    attempts = 50;
    while (attempts-- > 0) {
        if (!isAlivePosix(pid)) return 1;
        nanosleep(&req, NULL);
    }
    return !isAlivePosix(pid);
#endif
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            long long pid;
            if (!isNumberLL(argv[i], &pid)) {
                printf("PID \"%s\" is invalid input.\n", argv[i]);
                continue;
            }
            int ok = terminateProcess(pid);
            printf("Terminate PID %lld: %s\n", pid, ok ? "success" : "failed");
        }
    } else {
        // 5 test cases
#ifdef _WIN32
        long long self = (long long)GetCurrentProcessId();
#else
        long long self = (long long)getpid();
#endif
        long long tests[5] = { -5, 0, 1, self, 99999999LL };
        for (int i = 0; i < 5; ++i) {
            int ok = terminateProcess(tests[i]);
            printf("Test terminate PID %lld: %s\n", tests[i], ok ? "success" : "failed");
        }
    }
    return 0;
}