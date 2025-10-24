#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

static int parse_ll(const char* s, long long* out) {
    if (!s) return 0;
    char* end = NULL;
    errno = 0;
    long long v = strtoll(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    *out = v;
    return 1;
}

#ifdef _WIN32
int terminate_process(long long pid_ll) {
    if (pid_ll <= 0) return 0;
    DWORD pid = (DWORD)pid_ll;

    if (pid == GetCurrentProcessId()) return 0; // avoid self-termination
    if (pid <= 4) return 0; // avoid critical/system PIDs

    HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!h) {
        return 0; // possibly no permission or not found
    }

    BOOL ok = TerminateProcess(h, 1);
    if (!ok) {
        CloseHandle(h);
        return 0;
    }

    DWORD waitRes = WaitForSingleObject(h, 5000);
    CloseHandle(h);
    return (waitRes == WAIT_OBJECT_0) ? 1 : 0;
}
#else
static int is_alive(pid_t pid) {
    if (pid <= 0) return 0;
    int r = kill(pid, 0);
    if (r == 0) return 1;
    if (errno == ESRCH) return 0;
    return 1; // EPERM or other -> assume alive
}

static void sleep_ms(long ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

int terminate_process(long long pid_ll) {
    if (pid_ll <= 0) return 0;
    pid_t pid = (pid_t)pid_ll;

    if (pid == getpid()) return 0; // avoid self-termination
    if (pid <= 1) return 0;        // avoid init/system PID

    if (!is_alive(pid)) return 1;

    if (kill(pid, SIGTERM) != 0) {
        if (errno == ESRCH) return 1;
        if (errno == EPERM) return 0;
    }

    for (int i = 0; i < 40; ++i) {
        if (!is_alive(pid)) return 1;
        sleep_ms(50);
    }

    if (kill(pid, SIGKILL) != 0) {
        if (errno == ESRCH) return 1;
        if (errno == EPERM) return 0;
    }

    for (int i = 0; i < 60; ++i) {
        if (!is_alive(pid)) return 1;
        sleep_ms(50);
    }

    return is_alive(pid) ? 0 : 1;
}
#endif

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            long long pid;
            if (!parse_ll(argv[i], &pid)) {
                printf("Arg '%s' is not a valid integer PID\n", argv[i]);
                continue;
            }
            int ok = terminate_process(pid);
            printf("Terminate PID %lld: %s\n", pid, ok ? "SUCCESS/NOT ALIVE" : "FAILED/ALIVE");
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
        printf("No arguments provided. Running 5 demonstrative test cases.\n");
        for (int i = 0; i < 5; ++i) {
            int ok = terminate_process(tests[i]);
            printf("Terminate PID %lld: %s\n", tests[i], ok ? "SUCCESS/NOT ALIVE" : "FAILED/ALIVE");
        }
    }
    return 0;
}